#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE
#include "HandmadeMath.h"

#include "falcon.h"
#include "offscreen-sapp.glsl.h"

#define OFFSCREEN_SAMPLE_COUNT (4)

namespace {

class app : public falcon::application {
    void configure(sapp_desc &desc) override {
        desc.width = 800;
        desc.height = 600;
        desc.sample_count = 4;
        desc.window_title = "Offscreen Rendering (falcon app)";

        _rx = 0.f;
        _ry = 0.f;
        _vs_params = vs_params_t{};
    }

    struct {
        void init(const falcon::gfx::buffer &vbuf, const falcon::gfx::buffer &ibuf, const falcon::gfx::image &color_img) {
            using namespace falcon::gfx;

            /* default pass action: clear to blue-ish */
            _pass_action = make_pass_action_clear(0.0f, 0.25f, 1.0f);

            /* and another pipeline-state-object for the default pass */
            _pipeline = make_pipeline([](auto &_) {
                /* don't need to provide buffer stride or attr offsets, no gaps here */
                _.layout.attrs[ATTR_vs_default_pos].format = SG_VERTEXFORMAT_FLOAT3;
                _.layout.attrs[ATTR_vs_default_color0].format = SG_VERTEXFORMAT_FLOAT4;
                _.layout.attrs[ATTR_vs_default_uv0].format = SG_VERTEXFORMAT_FLOAT2;
                _.shader = make_shader(default_shader_desc());
                _.index_type = SG_INDEXTYPE_UINT16;
                _.depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL;
                _.depth_stencil.depth_write_enabled = true;
                _.rasterizer.cull_mode = SG_CULLMODE_BACK;
                _.label = "default-pipeline";
            });

            /* resource bindings to render a textured cube, using the offscreen render target as texture */
            _bindings = make<bindings>([&vbuf, &ibuf, &color_img](auto &_) {
                _.vertex_buffers[0] = vbuf;
                _.index_buffer = ibuf;
                _.fs_images[SLOT_tex] = color_img;
            });
        }

        void frame(const vs_params_t &vs_params, int width, int height) {
            falcon::gfx::begin(_pass_action, width, height)
                .pipeline(_pipeline)
                    .bindings(_bindings)
                    .uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &vs_params, sizeof(vs_params))
                    .draw(0, 36, 1);
        }

        falcon::gfx::pass_action _pass_action;
        falcon::gfx::pipeline _pipeline;
        falcon::gfx::bindings _bindings;
    } _default;
    
    struct {
        void init(const falcon::gfx::buffer &vbuf, const falcon::gfx::buffer &ibuf, const falcon::gfx::image &color_img) {
            using namespace falcon::gfx;

            /* offscreen pass action: clear to black */
            _pass_action = make_pass_action_clear(0.0f, 0.0f, 0.0f);

            /* a render pass with one color- and one depth-attachment image */
            auto depth_img = make_image([](auto &_) {
                _.render_target = true;
                _.width = 256;
                _.height = 256;
                _.pixel_format = SG_PIXELFORMAT_DEPTH;
                _.min_filter = SG_FILTER_LINEAR;
                _.mag_filter = SG_FILTER_LINEAR;
                _.sample_count = OFFSCREEN_SAMPLE_COUNT;
                _.label = "depth-image";
            });
            _pass = make_pass([&color_img, &depth_img](auto &_) {
                _.color_attachments[0].image = color_img;
                _.depth_stencil_attachment.image = depth_img;
                _.label = "offscreen-pass";
            });

            /* pipeline-state-object for offscreen-rendered cube, don't need texture coord here */
            _pipeline = make_pipeline([](auto &_) {
                /* need to provide stride, because the buffer's texcoord is skipped */
                _.layout.buffers[0].stride = 36;

                /* but don't need to provide attr offsets, because pos and color are continuous */
                _.layout.attrs[ATTR_vs_offscreen_pos].format = SG_VERTEXFORMAT_FLOAT3;
                _.layout.attrs[ATTR_vs_offscreen_color0].format = SG_VERTEXFORMAT_FLOAT4;
                _.shader = make_shader(offscreen_shader_desc());
                _.index_type = SG_INDEXTYPE_UINT16;
                _.depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL;
                _.depth_stencil.depth_write_enabled = true;
                _.blend.color_format = SG_PIXELFORMAT_RGBA8;
                _.blend.depth_format = SG_PIXELFORMAT_DEPTH;
                _.rasterizer.cull_mode = SG_CULLMODE_BACK;
                _.rasterizer.sample_count = OFFSCREEN_SAMPLE_COUNT;
                _.label = "offscreen-pipeline";
            });

            /* the resource bindings for rendering a non-textured cube into offscreen render target */
            _bindings = make<bindings>([&vbuf, &ibuf](auto &_) {
                _.vertex_buffers[0] = vbuf;
                _.index_buffer = ibuf;
            });
        }

        void frame(const vs_params_t &vs_params) {
            falcon::gfx::begin(_pass, _pass_action)
                .pipeline(_pipeline)
                    .bindings(_bindings)
                    .uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &vs_params, sizeof(vs_params))
                    .draw(0, 36, 1);
        }

        falcon::gfx::pass_action _pass_action;
        falcon::gfx::pass _pass;
        falcon::gfx::pipeline _pipeline;
        falcon::gfx::bindings _bindings;
    } _offscreen;

    void init() override {
        using namespace falcon::gfx;

        /* a render pass with one color- and one depth-attachment image */
        auto color_img = make_image([](auto &_) {
            _.render_target = true;
            _.width = 256;
            _.height = 256;
            _.pixel_format = SG_PIXELFORMAT_RGBA8;
            _.min_filter = SG_FILTER_LINEAR;
            _.mag_filter = SG_FILTER_LINEAR;
            _.sample_count = OFFSCREEN_SAMPLE_COUNT;
            _.label = "color-image";
        });

        /* cube vertex buffer with positions, colors and tex coords */
        float vertices[] = {
            /* pos                  color                       uvs */
            -1.0f, -1.0f, -1.0f,    1.0f, 0.5f, 0.5f, 1.0f,     0.0f, 0.0f,
             1.0f, -1.0f, -1.0f,    1.0f, 0.5f, 0.5f, 1.0f,     1.0f, 0.0f,
             1.0f,  1.0f, -1.0f,    1.0f, 0.5f, 0.5f, 1.0f,     1.0f, 1.0f,
            -1.0f,  1.0f, -1.0f,    1.0f, 0.5f, 0.5f, 1.0f,     0.0f, 1.0f,

            -1.0f, -1.0f,  1.0f,    0.5f, 1.0f, 0.5f, 1.0f,     0.0f, 0.0f,
             1.0f, -1.0f,  1.0f,    0.5f, 1.0f, 0.5f, 1.0f,     1.0f, 0.0f,
             1.0f,  1.0f,  1.0f,    0.5f, 1.0f, 0.5f, 1.0f,     1.0f, 1.0f,
            -1.0f,  1.0f,  1.0f,    0.5f, 1.0f, 0.5f, 1.0f,     0.0f, 1.0f,

            -1.0f, -1.0f, -1.0f,    0.5f, 0.5f, 1.0f, 1.0f,     0.0f, 0.0f,
            -1.0f,  1.0f, -1.0f,    0.5f, 0.5f, 1.0f, 1.0f,     1.0f, 0.0f,
            -1.0f,  1.0f,  1.0f,    0.5f, 0.5f, 1.0f, 1.0f,     1.0f, 1.0f,
            -1.0f, -1.0f,  1.0f,    0.5f, 0.5f, 1.0f, 1.0f,     0.0f, 1.0f,

             1.0f, -1.0f, -1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     0.0f, 0.0f,
             1.0f,  1.0f, -1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     1.0f, 0.0f,
             1.0f,  1.0f,  1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     1.0f, 1.0f,
             1.0f, -1.0f,  1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     0.0f, 1.0f,

            -1.0f, -1.0f, -1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     0.0f, 0.0f,
            -1.0f, -1.0f,  1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     1.0f, 0.0f,
             1.0f, -1.0f,  1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     1.0f, 1.0f,
             1.0f, -1.0f, -1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     0.0f, 1.0f,

            -1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     0.0f, 0.0f,
            -1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     1.0f, 0.0f,
             1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     1.0f, 1.0f,
             1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     0.0f, 1.0f
        };
        auto vbuf = make_vertex_buffer(vertices, sizeof(vertices), "cube-vertices");

        /* an index buffer for the cube */
        uint16_t indices[] = {
            0, 1, 2,  0, 2, 3,
            6, 5, 4,  7, 6, 4,
            8, 9, 10,  8, 10, 11,
            14, 13, 12,  15, 14, 12,
            16, 17, 18,  16, 18, 19,
            22, 21, 20,  23, 22, 20
        };
        auto ibuf = make_index_buffer(indices, sizeof(indices), "cube-indices");

        _default.init(vbuf, ibuf, color_img);
        _offscreen.init(vbuf, ibuf, color_img);
    }

    void frame() override {
        const float w = (float)width(), h = (float)height();

        /* compute model-view-projection matrix for vertex shader, this will be
        used both for the offscreen-pass, and the display-pass */
        hmm_mat4 proj = HMM_Perspective(60.0f, w/h, 0.01f, 10.0f);
        hmm_mat4 view = HMM_LookAt(HMM_Vec3(0.0f, 1.5f, 6.0f), HMM_Vec3(0.0f, 0.0f, 0.0f), HMM_Vec3(0.0f, 1.0f, 0.0f));
        hmm_mat4 view_proj = HMM_MultiplyMat4(proj, view);
        _rx += 1.0f; _ry += 2.0f;
        hmm_mat4 rxm = HMM_Rotate(_rx, HMM_Vec3(1.0f, 0.0f, 0.0f));
        hmm_mat4 rym = HMM_Rotate(_ry, HMM_Vec3(0.0f, 1.0f, 0.0f));
        hmm_mat4 model = HMM_MultiplyMat4(rxm, rym);
        _vs_params.mvp = HMM_MultiplyMat4(view_proj, model);

        /* the offscreen pass, rendering an rotating, untextured cube into a render target image */
        _offscreen.frame(_vs_params);

        /* and the display-pass, rendering a rotating, textured cube, using the
        previously rendered offscreen render-target as texture */
        _default.frame(_vs_params, width(), height());
    }

    vs_params_t _vs_params;
    float _rx, _ry;
};

} // namespace

FALCON_MAIN(::app);
