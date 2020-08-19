#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE
#include "HandmadeMath.h"

#include "falcon.h"
#include "mrt-sapp.glsl.h"

#define OFFSCREEN_SAMPLE_COUNT (4)

namespace {

class app : public falcon::application {
    void configure(sapp_desc &desc) override {
        desc.width = 800;
        desc.height = 600;
        desc.sample_count = 4;
        desc.window_title = "MRT Rendering (falcon app)";

        _rx = 0.f;
        _ry = 0.f;
        _offscreen = {};
        _fsq = {};
        _dbg = {};
    }

    typedef struct {
        float x, y, z, b;
    } vertex_t;

    struct {
        void init() {
            using namespace falcon::gfx;

            /* resource bindings for offscreen rendering */
            _bindings = make<bindings>([](auto &_) {
                /* cube vertex buffer */
                vertex_t cube_vertices[] = {
                    /* pos + brightness */
                    { -1.0f, -1.0f, -1.0f,   1.0f },
                    {  1.0f, -1.0f, -1.0f,   1.0f },
                    {  1.0f,  1.0f, -1.0f,   1.0f },
                    { -1.0f,  1.0f, -1.0f,   1.0f },

                    { -1.0f, -1.0f,  1.0f,   0.8f },
                    {  1.0f, -1.0f,  1.0f,   0.8f },
                    {  1.0f,  1.0f,  1.0f,   0.8f },
                    { -1.0f,  1.0f,  1.0f,   0.8f },

                    { -1.0f, -1.0f, -1.0f,   0.6f },
                    { -1.0f,  1.0f, -1.0f,   0.6f },
                    { -1.0f,  1.0f,  1.0f,   0.6f },
                    { -1.0f, -1.0f,  1.0f,   0.6f },

                    { 1.0f, -1.0f, -1.0f,    0.4f },
                    { 1.0f,  1.0f, -1.0f,    0.4f },
                    { 1.0f,  1.0f,  1.0f,    0.4f },
                    { 1.0f, -1.0f,  1.0f,    0.4f },

                    { -1.0f, -1.0f, -1.0f,   0.5f },
                    { -1.0f, -1.0f,  1.0f,   0.5f },
                    {  1.0f, -1.0f,  1.0f,   0.5f },
                    {  1.0f, -1.0f, -1.0f,   0.5f },

                    { -1.0f,  1.0f, -1.0f,   0.7f },
                    { -1.0f,  1.0f,  1.0f,   0.7f },
                    {  1.0f,  1.0f,  1.0f,   0.7f },
                    {  1.0f,  1.0f, -1.0f,   0.7f },
                };
                _.vertex_buffers[0] = make_vertex_buffer(cube_vertices, sizeof(cube_vertices), "cube vertices");

                /* index buffer for the cube */
                uint16_t cube_indices[] = {
                    0, 1, 2,  0, 2, 3,
                    6, 5, 4,  7, 6, 4,
                    8, 9, 10,  8, 10, 11,
                    14, 13, 12,  15, 14, 12,
                    16, 17, 18,  16, 18, 19,
                    22, 21, 20,  23, 22, 20
                };
                _.index_buffer = make_index_buffer(cube_indices, sizeof(cube_indices), "cube indices");
            });

            /* pass action for offscreen pass */
            _pass_action = make<pass_action>([](auto &_) {
                _.colors[0] = make<sg_color_attachment_action>([](auto &_) {
                    _.action = SG_ACTION_CLEAR;
                    _.val[0] = 0.25f;
                    _.val[1] = 0.0f;
                    _.val[2] = 0.0f;
                    _.val[3] = 1.0f;
                });
                _.colors[1] = make<sg_color_attachment_action>([](auto &_) {
                    _.action = SG_ACTION_CLEAR;
                    _.val[0] = 0.0f;
                    _.val[1] = 0.25f;
                    _.val[2] = 0.0f;
                    _.val[3] = 1.0f;
                });
                _.colors[2] = make<sg_color_attachment_action>([](auto &_) {
                    _.action = SG_ACTION_CLEAR;
                    _.val[0] = 0.0f;
                    _.val[1] = 0.0f;
                    _.val[2] = 0.25f;
                    _.val[3] = 1.0f;
                });
            });

            /* pipeline object for the offscreen-rendered cube */
            _pipeline = make_pipeline([](auto &_) {
                _.layout = make<sg_layout_desc>([](auto &_){
                    _.buffers[0].stride = sizeof(vertex_t);
                    _.attrs[ATTR_vs_offscreen_pos] = make<sg_vertex_attr_desc>([](auto &_) {
                        _.offset = offsetof(vertex_t, x);
                        _.format = SG_VERTEXFORMAT_FLOAT3;
                    });
                    _.attrs[ATTR_vs_offscreen_bright0] = make<sg_vertex_attr_desc>([](auto &_) {
                        _.offset = offsetof(vertex_t, b);
                        _.format = SG_VERTEXFORMAT_FLOAT;
                    });
                });
                /* a shader to render the cube into offscreen MRT render targest */
                _.shader = make_shader(offscreen_shader_desc());
                _.index_type = SG_INDEXTYPE_UINT16;
                _.depth_stencil = make<sg_depth_stencil_state>([](auto &_) {
                    _.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL;
                    _.depth_write_enabled = true;
                });
                _.blend = make<sg_blend_state>([](auto &_) {
                    _.color_attachment_count = 3;
                    _.depth_format = SG_PIXELFORMAT_DEPTH;
                });
                _.rasterizer = make<sg_rasterizer_state>([](auto &_) {
                    _.cull_mode = SG_CULLMODE_BACK;
                    _.sample_count = OFFSCREEN_SAMPLE_COUNT;
                });
                _.label = "offscreen pipeline";
            });
        }

        /* render cube into MRT offscreen render targets */
        void frame() {
            falcon::gfx::begin(_pass, _pass_action)
                .pipeline(_pipeline)
                    .bindings(_bindings)
                    .uniforms(SG_SHADERSTAGE_VS, SLOT_offscreen_params, &_params, sizeof(_params))
                    .draw(0, 36, 1)
            ;
        }

        offscreen_params_t _params;

        falcon::gfx::pass_action _pass_action;
        sg_pass_desc _pass_desc;
        falcon::gfx::pass _pass;
        falcon::gfx::pipeline _pipeline;
        falcon::gfx::bindings _bindings;
    } _offscreen;

    struct {
        void init(const falcon::gfx::buffer &quad_vbuf, const sg_pass_desc &pass_desc) {
            using namespace falcon::gfx;

            /* the pipeline object to render the fullscreen quad */
            _pipeline = make_pipeline([](auto &_) {
                _.layout.attrs[ATTR_vs_fsq_pos].format = SG_VERTEXFORMAT_FLOAT2;
                _.shader = make_shader(fsq_shader_desc());
                _.primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP;
                _.label = "fullscreen quad pipeline";
            });

            /* resource bindings to render a fullscreen quad */
            _bindings = make<bindings>([&quad_vbuf, &pass_desc](auto &_) {
                _.vertex_buffers[0] = quad_vbuf;
                _.fs_images[SLOT_tex0] = pass_desc.color_attachments[0].image;
                _.fs_images[SLOT_tex1] = pass_desc.color_attachments[1].image;
                _.fs_images[SLOT_tex2] = pass_desc.color_attachments[2].image;
            });
        }

        fsq_params_t _params;

        falcon::gfx::pipeline _pipeline;
        falcon::gfx::bindings _bindings;
    } _fsq;

    struct {
        void init(const falcon::gfx::buffer &quad_vbuf) {
            using namespace falcon::gfx;

            /* pipeline and resource bindings to render debug-visualization quads */
            _pipeline = make_pipeline([](auto &_) {
                _.layout.attrs[ATTR_vs_dbg_pos].format = SG_VERTEXFORMAT_FLOAT2;
                _.primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP;
                _.shader = make_shader(dbg_shader_desc());
                _.label = "dbgvis quad pipeline";
            });

            /* resource bindings to render a fullscreen quad */
            _bindings = make<bindings>([&quad_vbuf](auto &_) {
                _.vertex_buffers[0] = quad_vbuf;
                /* images will be filled right before rendering */
            });
        }

        falcon::gfx::pipeline _pipeline;
        falcon::gfx::bindings _bindings;
    } _dbg;

    /* called initially and when window size changes */
    void create_offscreen_pass(int width, int height) {
        using namespace falcon::gfx;

        /* destroy previous resource (can be called for invalid id) */
        destroy(_offscreen._pass);
        for (auto &color : _offscreen._pass_desc.color_attachments) {
            destroy(color.image);
        }
        destroy(_offscreen._pass_desc.depth_stencil_attachment.image);

        /* create offscreen rendertarget images and pass */
        const int offscreen_sample_count = sg_query_features().msaa_render_targets ? OFFSCREEN_SAMPLE_COUNT : 1;
        auto color_img_desc = make<sg_image_desc>([&width, &height, &offscreen_sample_count](auto &_) {
            _.render_target = true;
            _.width = width;
            _.height = height;
            _.min_filter = SG_FILTER_LINEAR;
            _.mag_filter = SG_FILTER_LINEAR;
            _.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
            _.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
            _.sample_count = offscreen_sample_count;
            _.label = "color image";
        });
        auto depth_img_desc = color_img_desc;
        depth_img_desc.pixel_format = SG_PIXELFORMAT_DEPTH;
        depth_img_desc.label = "depth image";
        _offscreen._pass_desc = make<sg_pass_desc>([&color_img_desc, &depth_img_desc](auto &_) {
            _.color_attachments[0].image = make_image(color_img_desc);
            _.color_attachments[1].image = make_image(color_img_desc);
            _.color_attachments[2].image = make_image(color_img_desc);
            _.depth_stencil_attachment.image = make_image(depth_img_desc);
            _.label = "offscreen pass";
        });
        _offscreen._pass = make_pass(_offscreen._pass_desc);

        /* also need to update the fullscreen-quad texture bindings */
        for (int i = 0; i < 3; i++) {
            _fsq._bindings.fs_images[i] = _offscreen._pass_desc.color_attachments[i].image;
        }
    }

    /* listen for window-resize events and recreate offscreen rendertargets */
    void event(const sapp_event* e) override {
        if (e->type == SAPP_EVENTTYPE_RESIZED) {
            create_offscreen_pass(e->framebuffer_width, e->framebuffer_height);
        }
    }

    void init() override {
        using namespace falcon::gfx;

        if (sapp_gles2()) {
            /* this demo needs GLES3/WebGL */
            _pass_action = make_pass_action_clear(1.0f, 0.0f, 0.0f);
            return;
        }

        /* a pass action for the default render pass */
        _pass_action = make<pass_action>([](auto &_) {
            _.colors[0].action = SG_ACTION_DONTCARE;
            _.depth.action = SG_ACTION_DONTCARE;
            _.stencil.action = SG_ACTION_DONTCARE;
        });

        /* a render pass with 3 color attachment images, and a depth attachment image */
        create_offscreen_pass(width(), height());

        _offscreen.init();

        /* a vertex buffer to render a fullscreen rectangle */
        float quad_vertices[] = { 0.0f, 0.0f,  1.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f };
        auto quad_vbuf = make_vertex_buffer(quad_vertices, sizeof(quad_vertices), "quad vertices");

        _fsq.init(quad_vbuf, _offscreen._pass_desc);
        _dbg.init(quad_vbuf);
    }

    /* this is called when GLES3/WebGL2 is not available */
    void draw_gles2_fallback(void) {
        falcon::gfx::begin(_pass_action, width(), height());
    }

    void frame() override {
        /* can't do anything useful on GLES2/WebGL */
        if (sapp_gles2()) {
            draw_gles2_fallback();
            return;
        }

        const float w = (float)width(), h = (float)height();

        /* view-projection matrix */
        hmm_mat4 proj = HMM_Perspective(60.0f, w/h, 0.01f, 10.0f);
        hmm_mat4 view = HMM_LookAt(HMM_Vec3(0.0f, 1.5f, 6.0f), HMM_Vec3(0.0f, 0.0f, 0.0f), HMM_Vec3(0.0f, 1.0f, 0.0f));
        hmm_mat4 view_proj = HMM_MultiplyMat4(proj, view);

        /* shader parameters */
        _rx += 1.0f; _ry += 2.0f;
        hmm_mat4 rxm = HMM_Rotate(_rx, HMM_Vec3(1.0f, 0.0f, 0.0f));
        hmm_mat4 rym = HMM_Rotate(_ry, HMM_Vec3(0.0f, 1.0f, 0.0f));
        hmm_mat4 model = HMM_MultiplyMat4(rxm, rym);
        _offscreen._params.mvp = HMM_MultiplyMat4(view_proj, model);
        _fsq._params.offset = HMM_Vec2(HMM_SinF(_rx*0.01f)*0.1f, HMM_SinF(_ry*0.01f)*0.1f);

        _offscreen.frame();

        /* render fullscreen quad with the 'composed image', plus 3
           small debug-view quads */
        falcon::gfx::begin(_pass_action, width(), height())
            .pipeline(_fsq._pipeline)
                .bindings(_fsq._bindings)
                .uniforms(SG_SHADERSTAGE_VS, SLOT_fsq_params, &_fsq._params, sizeof(_fsq._params))
                .draw(0, 4, 1)
            .pipeline(_dbg._pipeline)
            .apply([this](auto &_) {
                for (int i = 0; i < 3; i++) {
                    _dbg._bindings.fs_images[SLOT_tex] = _offscreen._pass_desc.color_attachments[i].image;
                    _.viewport(i*100, 0, 100, 100, false)
                        .bindings(_dbg._bindings)
                        .draw(0, 4, 1);
                }
            })
            .viewport(0, 0, width(), height(), false);
    }

    float _rx, _ry;
    falcon::gfx::pass_action _pass_action;
};

} // namespace

FALCON_MAIN(::app);
