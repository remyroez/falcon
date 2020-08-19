
#include <stdlib.h> /* rand */

#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE
#include "HandmadeMath.h"

#include "falcon.h"
#include "dyntex-sapp.glsl.h"

#define IMAGE_WIDTH (64)
#define IMAGE_HEIGHT (64)
#define LIVING (0xFFFFFFFF)
#define DEAD (0xFF000000)

namespace {

class app : public falcon::application {
    void configure(sapp_desc &desc) override {
        desc.width = 800;
        desc.height = 600;
        desc.sample_count = 4;
        desc.window_title = "Textured Cube (falcon app)";

        _rx = 0.f;
        _ry = 0.f;
        _update_count = 0;

        _vs_params = {};
    }

    void init() override {
        using namespace falcon::gfx;

        _bindings = make<bindings>([](auto &_) {
            /* cube vertex buffer */
            float vertices[] = {
                /* pos                  color                       uvs */
                -1.0f, -1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,     0.0f, 0.0f,
                 1.0f, -1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,     1.0f, 0.0f,
                 1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,     1.0f, 1.0f,
                -1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,     0.0f, 1.0f,

                -1.0f, -1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,     0.0f, 0.0f,
                 1.0f, -1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,     1.0f, 0.0f,
                 1.0f,  1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,     1.0f, 1.0f,
                -1.0f,  1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,     0.0f, 1.0f,

                -1.0f, -1.0f, -1.0f,    0.0f, 0.0f, 1.0f, 1.0f,     0.0f, 0.0f,
                -1.0f,  1.0f, -1.0f,    0.0f, 0.0f, 1.0f, 1.0f,     1.0f, 0.0f,
                -1.0f,  1.0f,  1.0f,    0.0f, 0.0f, 1.0f, 1.0f,     1.0f, 1.0f,
                -1.0f, -1.0f,  1.0f,    0.0f, 0.0f, 1.0f, 1.0f,     0.0f, 1.0f,

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
            _.vertex_buffers[0] = make_vertex_buffer(vertices, sizeof(vertices), "cube-vertices");

            uint16_t indices[] = {
                0, 1, 2,  0, 2, 3,
                6, 5, 4,  7, 6, 4,
                8, 9, 10,  8, 10, 11,
                14, 13, 12,  15, 14, 12,
                16, 17, 18,  16, 18, 19,
                22, 21, 20,  23, 22, 20
            };
            _.index_buffer = make_index_buffer(indices, sizeof(indices), "cube-indices");

            /* a 128x128 image with streaming update strategy */
            _.fs_images[SLOT_tex] = make_image([](auto &_) {
                _.width = IMAGE_WIDTH;
                _.height = IMAGE_HEIGHT;
                _.pixel_format = SG_PIXELFORMAT_RGBA8;
                _.usage = SG_USAGE_STREAM;
                _.min_filter = SG_FILTER_LINEAR;
                _.mag_filter = SG_FILTER_LINEAR;
                _.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
                _.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
                _.label = "dynamic-texture";
            });
        });

        /* a pipeline state object */
        _pipeline = make_pipeline([](auto &_) {
            _.layout.attrs[ATTR_vs_position].format = SG_VERTEXFORMAT_FLOAT3;
            _.layout.attrs[ATTR_vs_color0].format = SG_VERTEXFORMAT_FLOAT4;
            _.layout.attrs[ATTR_vs_texcoord0].format = SG_VERTEXFORMAT_FLOAT2;
            /* a shader to render a textured cube */
            _.shader = make_shader(dyntex_shader_desc());
            _.index_type = SG_INDEXTYPE_UINT16;
            _.depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL;
            _.depth_stencil.depth_write_enabled = true;
            _.rasterizer.cull_mode = SG_CULLMODE_BACK;
            _.label = "cube-pipeline";
        });

        /* initialize the game-of-life state */
        game_of_life_init();
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

        /* update game-of-life state */
        game_of_life_update();

        /* update the texture */
        falcon::gfx::update_image(_bindings.fs_images[0], [this](auto &_) {
            _.subimage[0][0].ptr = _pixels;
            _.subimage[0][0].size = sizeof(_pixels);
        });

        /* render the frame */
        falcon::gfx::begin(_pass_action, w, h)
            .pipeline(_pipeline)
                .bindings(_bindings)
                .uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &_vs_params, sizeof(_vs_params))
                .draw(0, 36, 1);
    }

    void game_of_life_init() {
        for (int y = 0; y < IMAGE_HEIGHT; y++) {
            for (int x = 0; x < IMAGE_WIDTH; x++) {
                if ((rand() & 255) > 230) {
                    _pixels[y][x] = LIVING;
                }
                else {
                    _pixels[y][x] = DEAD;
                }
            }
        }
    }

    void game_of_life_update() {
        for (int y = 0; y < IMAGE_HEIGHT; y++) {
            for (int x = 0; x < IMAGE_WIDTH; x++) {
                int num_living_neighbours = 0;
                for (int ny = -1; ny < 2; ny++) {
                    for (int nx = -1; nx < 2; nx++) {
                        if ((nx == 0) && (ny == 0)) {
                            continue;
                        }
                        if (_pixels[(y+ny)&(IMAGE_HEIGHT-1)][(x+nx)&(IMAGE_WIDTH-1)] == LIVING) {
                            num_living_neighbours++;
                        }
                    }
                }
                /* any live cell... */
                if (_pixels[y][x] == LIVING) {
                    if (num_living_neighbours < 2) {
                        /* ... with fewer than 2 living neighbours dies, as if caused by underpopulation */
                        _pixels[y][x] = DEAD;
                    }
                    else if (num_living_neighbours > 3) {
                        /* ... with more than 3 living neighbours dies, as if caused by overpopulation */
                        _pixels[y][x] = DEAD;
                    }
                }
                else if (num_living_neighbours == 3) {
                    /* any dead cell with exactly 3 living neighbours becomes a live cell, as if by reproduction */
                    _pixels[y][x] = LIVING;
                }
            }
        }
        if (_update_count++ > 240) {
            game_of_life_init();
            _update_count = 0;
        }
    }

    float _rx, _ry;
    int _update_count;
    uint32_t _pixels[IMAGE_WIDTH][IMAGE_HEIGHT];

    vs_params_t _vs_params;

    falcon::gfx::pass_action _pass_action;
    falcon::gfx::pipeline _pipeline;
    falcon::gfx::bindings _bindings;
};

} // namespace

FALCON_MAIN(::app);
