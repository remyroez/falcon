#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE
#include "HandmadeMath.h"

#include "falcon.h"
#include "arraytex-sapp.glsl.h"

#define IMG_LAYERS (3)
#define IMG_WIDTH (16)
#define IMG_HEIGHT (16)

namespace {

class app : public falcon::application {
    void configure(sapp_desc &desc) override {
        desc.width = 800;
        desc.height = 600;
        desc.sample_count = 4;
        desc.window_title = "Array Texture (falcon app)";

        _rx = 0.f;
        _ry = 0.f;
        _vs_params = vs_params_t{};
        _frame_index = 0;
    }

    typedef struct {
        float x, y, z;
        uint32_t color;
        int16_t u, v;
    } vertex_t;

    void init() override {
        using namespace falcon::gfx;

        if (sapp_gles2()) {
            /* this demo needs GLES3/WebGL */
            _pass_action = make_pass_action_clear(1.0f, 0.0f, 0.0f);
            return;
        }

        /* a default pass action to clear to black */
        _pass_action = make_pass_action_clear(0.0f, 0.0f, 0.0f);

        /* populate the resource bindings struct */
        _bindings = make<bindings>([](auto &_) {
            /* cube vertex buffer */
            float vertices[] = {
                /* pos                  uvs */
                -1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
                 1.0f, -1.0f, -1.0f,    1.0f, 0.0f,
                 1.0f,  1.0f, -1.0f,    1.0f, 1.0f,
                -1.0f,  1.0f, -1.0f,    0.0f, 1.0f,

                -1.0f, -1.0f,  1.0f,    0.0f, 0.0f,
                 1.0f, -1.0f,  1.0f,    1.0f, 0.0f,
                 1.0f,  1.0f,  1.0f,    1.0f, 1.0f,
                -1.0f,  1.0f,  1.0f,    0.0f, 1.0f,

                -1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
                -1.0f,  1.0f, -1.0f,    1.0f, 0.0f,
                -1.0f,  1.0f,  1.0f,    1.0f, 1.0f,
                -1.0f, -1.0f,  1.0f,    0.0f, 1.0f,

                 1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
                 1.0f,  1.0f, -1.0f,    1.0f, 0.0f,
                 1.0f,  1.0f,  1.0f,    1.0f, 1.0f,
                 1.0f, -1.0f,  1.0f,    0.0f, 1.0f,

                -1.0f, -1.0f, -1.0f,    0.0f, 0.0f,
                -1.0f, -1.0f,  1.0f,    1.0f, 0.0f,
                 1.0f, -1.0f,  1.0f,    1.0f, 1.0f,
                 1.0f, -1.0f, -1.0f,    0.0f, 1.0f,

                -1.0f,  1.0f, -1.0f,    0.0f, 0.0f,
                -1.0f,  1.0f,  1.0f,    1.0f, 0.0f,
                 1.0f,  1.0f,  1.0f,    1.0f, 1.0f,
                 1.0f,  1.0f, -1.0f,    0.0f, 1.0f
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

            /* a 16x16 array texture with 3 layers and a checkerboard pattern */
            static uint32_t pixels[IMG_LAYERS][IMG_HEIGHT][IMG_WIDTH];
            for (int layer=0, even_odd=0; layer<IMG_LAYERS; layer++) {
                for (int y = 0; y < IMG_HEIGHT; y++, even_odd++) {
                    for (int x = 0; x < IMG_WIDTH; x++, even_odd++) {
                        if (even_odd & 1) {
                            switch (layer) {
                                case 0: pixels[layer][y][x] = 0x000000FF; break;
                                case 1: pixels[layer][y][x] = 0x0000FF00; break;
                                case 2: pixels[layer][y][x] = 0x00FF0000; break;
                            }
                        }
                        else {
                            pixels[layer][y][x] = 0;
                        }
                    }
                }
            }
            _.fs_images[SLOT_tex] = make_image([](auto &_) {
                _.type = SG_IMAGETYPE_ARRAY;
                _.width = IMG_WIDTH;
                _.height = IMG_HEIGHT;
                _.layers = IMG_LAYERS;
                _.pixel_format = SG_PIXELFORMAT_RGBA8;
                _.min_filter = SG_FILTER_LINEAR;
                _.mag_filter = SG_FILTER_LINEAR;
                _.content.subimage[0][0].ptr = pixels;
                _.content.subimage[0][0].size = sizeof(pixels);
                _.label = "array-texture";
            });
        });

        /* a pipeline object */
        _pipeline = make_pipeline([](auto &_) {
            _.layout.attrs[ATTR_vs_position].format = SG_VERTEXFORMAT_FLOAT3;
            _.layout.attrs[ATTR_vs_texcoord0].format = SG_VERTEXFORMAT_FLOAT2;
            _.shader = make_shader(arraytex_shader_desc());
            _.index_type = SG_INDEXTYPE_UINT16;
            _.depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL;
            _.depth_stencil.depth_write_enabled = true;
            _.rasterizer.cull_mode = SG_CULLMODE_NONE;
            _.label = "cube-pipeline";
        });
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

        /* rotated model matrix */
        hmm_mat4 proj = HMM_Perspective(60.0f, w/h, 0.01f, 10.0f);
        hmm_mat4 view = HMM_LookAt(HMM_Vec3(0.0f, 1.5f, 6.0f), HMM_Vec3(0.0f, 0.0f, 0.0f), HMM_Vec3(0.0f, 1.0f, 0.0f));
        hmm_mat4 view_proj = HMM_MultiplyMat4(proj, view);
        _rx += 0.25f; _ry += 0.5f;
        hmm_mat4 rxm = HMM_Rotate(_rx, HMM_Vec3(1.0f, 0.0f, 0.0f));
        hmm_mat4 rym = HMM_Rotate(_ry, HMM_Vec3(0.0f, 1.0f, 0.0f));
        hmm_mat4 model = HMM_MultiplyMat4(rxm, rym);

        /* model-view-projection matrix for vertex shader */
        _vs_params.mvp = HMM_MultiplyMat4(view_proj, model);
        /* uv offsets */
        float offset = (float) _frame_index * 0.0001f;
        _vs_params.offset0 = HMM_Vec2(-offset, offset);
        _vs_params.offset1 = HMM_Vec2(offset, -offset);
        _vs_params.offset2 = HMM_Vec2(0.0f, 0.0f);
        _frame_index++;

        falcon::gfx::begin(_pass_action, w, h)
            .pipeline(_pipeline)
                .bindings(_bindings)
                .uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &_vs_params, sizeof(_vs_params))
                .draw(0, 36, 1);
    }

    vs_params_t _vs_params;
    float _rx, _ry;
    int _frame_index;

    falcon::gfx::pass_action _pass_action;
    falcon::gfx::pipeline _pipeline;
    falcon::gfx::bindings _bindings;
};

} // namespace

FALCON_MAIN(::app);
