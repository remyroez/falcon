#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE
#include "HandmadeMath.h"

#include "falcon.h"
#include "texcube-sapp.glsl.h"

namespace {

class app : public falcon::application {
    void configure(sapp_desc &desc) override {
        desc.width = 800;
        desc.height = 600;
        desc.sample_count = 4;
        desc.window_title = "Textured Cube (falcon app)";

        _rx = 0.f;
        _ry = 0.f;
        _vs_params = vs_params_t{};
    }

    typedef struct {
        float x, y, z;
        uint32_t color;
        int16_t u, v;
    } vertex_t;

    void init() override {
        using namespace falcon::gfx;

        _bindings = make<bindings>([](auto &_) {
            /*
                Cube vertex buffer with packed vertex formats for color and texture coords.
                Note that a vertex format which must be portable across all
                backends must only use the normalized integer formats
                (BYTE4N, UBYTE4N, SHORT2N, SHORT4N), which can be converted
                to floating point formats in the vertex shader inputs.

                The reason is that D3D11 cannot convert from non-normalized
                formats to floating point inputs (only to integer inputs),
                and WebGL2 / GLES2 don't support integer vertex shader inputs.
            */
            vertex_t vertices[] = {
                /* pos                  color       uvs */
                { -1.0f, -1.0f, -1.0f,  0xFF0000FF,     0,     0 },
                {  1.0f, -1.0f, -1.0f,  0xFF0000FF, 32767,     0 },
                {  1.0f,  1.0f, -1.0f,  0xFF0000FF, 32767, 32767 },
                { -1.0f,  1.0f, -1.0f,  0xFF0000FF,     0, 32767 },

                { -1.0f, -1.0f,  1.0f,  0xFF00FF00,     0,     0 },
                {  1.0f, -1.0f,  1.0f,  0xFF00FF00, 32767,     0 },
                {  1.0f,  1.0f,  1.0f,  0xFF00FF00, 32767, 32767 },
                { -1.0f,  1.0f,  1.0f,  0xFF00FF00,     0, 32767 },

                { -1.0f, -1.0f, -1.0f,  0xFFFF0000,     0,     0 },
                { -1.0f,  1.0f, -1.0f,  0xFFFF0000, 32767,     0 },
                { -1.0f,  1.0f,  1.0f,  0xFFFF0000, 32767, 32767 },
                { -1.0f, -1.0f,  1.0f,  0xFFFF0000,     0, 32767 },

                {  1.0f, -1.0f, -1.0f,  0xFFFF007F,     0,     0 },
                {  1.0f,  1.0f, -1.0f,  0xFFFF007F, 32767,     0 },
                {  1.0f,  1.0f,  1.0f,  0xFFFF007F, 32767, 32767 },
                {  1.0f, -1.0f,  1.0f,  0xFFFF007F,     0, 32767 },

                { -1.0f, -1.0f, -1.0f,  0xFFFF7F00,     0,     0 },
                { -1.0f, -1.0f,  1.0f,  0xFFFF7F00, 32767,     0 },
                {  1.0f, -1.0f,  1.0f,  0xFFFF7F00, 32767, 32767 },
                {  1.0f, -1.0f, -1.0f,  0xFFFF7F00,     0, 32767 },

                { -1.0f,  1.0f, -1.0f,  0xFF007FFF,     0,     0 },
                { -1.0f,  1.0f,  1.0f,  0xFF007FFF, 32767,     0 },
                {  1.0f,  1.0f,  1.0f,  0xFF007FFF, 32767, 32767 },
                {  1.0f,  1.0f, -1.0f,  0xFF007FFF,     0, 32767 },
            };
            _.vertex_buffers[0] = make_vertex_buffer(vertices, sizeof(vertices), "cube-vertices");

            /* create an index buffer for the cube */
            uint16_t indices[] = {
                0, 1, 2,  0, 2, 3,
                6, 5, 4,  7, 6, 4,
                8, 9, 10,  8, 10, 11,
                14, 13, 12,  15, 14, 12,
                16, 17, 18,  16, 18, 19,
                22, 21, 20,  23, 22, 20
            };
            _.index_buffer = make_index_buffer(indices, sizeof(indices), "cube-indices");

            /* create a checkerboard texture */
            uint32_t pixels[4*4] = {
                0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF, 0xFF000000,
                0xFF000000, 0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF,
                0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF, 0xFF000000,
                0xFF000000, 0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF,
            };
            /* NOTE: tex_slot is provided by shader code generation */
            _.fs_images[SLOT_tex] = make_image([&pixels](auto &_) {
                _.width = 4;
                _.height = 4;
                {
                    auto &subimage = _.content.subimage[0][0];
                    subimage.ptr = pixels;
                    subimage.size = sizeof(pixels);
                }
                _.label = "cube-texture";
            });
        });

        /*
            a pipeline object, note that we need to provide the
            MSAA sample count of the default framebuffer
        */
        _pipeline = make_pipeline([](auto &_) {
            /* a shader */
            _.shader = make_shader(texcube_shader_desc());
            _.layout.attrs[ATTR_vs_pos].format = SG_VERTEXFORMAT_FLOAT3;
            _.layout.attrs[ATTR_vs_color0].format = SG_VERTEXFORMAT_UBYTE4N;
            _.layout.attrs[ATTR_vs_texcoord0].format = SG_VERTEXFORMAT_SHORT2N;
            _.index_type = SG_INDEXTYPE_UINT16;
            _.depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL;
            _.depth_stencil.depth_write_enabled = true;
            _.rasterizer.cull_mode = SG_CULLMODE_BACK;
            _.label = "cube-pipeline";
        });

        /* default pass action */
        _pass_action = make_pass_action_clear(0.25f, 0.5f, 0.75f);
    }

    void frame() override {
        const float w = (float)width(), h = (float)height();

        /* compute model-view-projection matrix for vertex shader */
        hmm_mat4 proj = HMM_Perspective(60.0f, w/h, 0.01f, 10.0f);
        hmm_mat4 view = HMM_LookAt(HMM_Vec3(0.0f, 1.5f, 6.0f), HMM_Vec3(0.0f, 0.0f, 0.0f), HMM_Vec3(0.0f, 1.0f, 0.0f));
        hmm_mat4 view_proj = HMM_MultiplyMat4(proj, view);
        _rx += 1.0f; _ry += 2.0f;
        hmm_mat4 rxm = HMM_Rotate(_rx, HMM_Vec3(1.0f, 0.0f, 0.0f));
        hmm_mat4 rym = HMM_Rotate(_ry, HMM_Vec3(0.0f, 1.0f, 0.0f));
        hmm_mat4 model = HMM_MultiplyMat4(rxm, rym);
        _vs_params.mvp = HMM_MultiplyMat4(view_proj, model);

        falcon::gfx::begin(_pass_action, w, h)
            .pipeline(_pipeline)
                .bindings(_bindings)
                .uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &_vs_params, sizeof(_vs_params))
                .draw(0, 36, 1);
    }

    vs_params_t _vs_params;
    float _rx, _ry;
    falcon::gfx::pass_action _pass_action;
    falcon::gfx::pipeline _pipeline;
    falcon::gfx::bindings _bindings;
};

} // namespace

FALCON_MAIN(::app);
