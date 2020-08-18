#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE
#include "HandmadeMath.h"

#include "falcon.h"
#include "noninterleaved-sapp.glsl.h"

namespace {

class app : public falcon::application {
    void configure(sapp_desc &desc) override {
        desc.width = 800;
        desc.height = 600;
        desc.sample_count = 4;
        desc.window_title = "Noninterleaved (falcon app)";

        _rx = 0.f;
        _ry = 0.f;
        _vs_params = vs_params_t{};
    }

    void init() override {
        using namespace falcon::gfx;

        _bindings = make<bindings>([](auto &_) {
            /* cube vertex buffer */
            float vertices[] = {
                /* positions */
                -1.0, -1.0, -1.0,   1.0, -1.0, -1.0,   1.0,  1.0, -1.0,  -1.0,  1.0, -1.0,
                -1.0, -1.0,  1.0,   1.0, -1.0,  1.0,   1.0,  1.0,  1.0,  -1.0,  1.0,  1.0,
                -1.0, -1.0, -1.0,  -1.0,  1.0, -1.0,  -1.0,  1.0,  1.0,  -1.0, -1.0,  1.0,
                 1.0, -1.0, -1.0,   1.0,  1.0, -1.0,   1.0,  1.0,  1.0,   1.0, -1.0,  1.0,
                -1.0, -1.0, -1.0,  -1.0, -1.0,  1.0,   1.0, -1.0,  1.0,   1.0, -1.0, -1.0,
                -1.0,  1.0, -1.0,  -1.0,  1.0,  1.0,   1.0,  1.0,  1.0,   1.0,  1.0, -1.0,

                /* colors */
                1.0, 0.5, 0.0, 1.0,  1.0, 0.5, 0.0, 1.0,  1.0, 0.5, 0.0, 1.0,  1.0, 0.5, 0.0, 1.0,
                0.5, 1.0, 0.0, 1.0,  0.5, 1.0, 0.0, 1.0,  0.5, 1.0, 0.0, 1.0,  0.5, 1.0, 0.0, 1.0,
                0.5, 0.0, 1.0, 1.0,  0.5, 0.0, 1.0, 1.0,  0.5, 0.0, 1.0, 1.0,  0.5, 0.0, 1.0, 1.0,
                1.0, 0.5, 1.0, 1.0,  1.0, 0.5, 1.0, 1.0,  1.0, 0.5, 1.0, 1.0,  1.0, 0.5, 1.0, 1.0,
                0.5, 1.0, 1.0, 1.0,  0.5, 1.0, 1.0, 1.0,  0.5, 1.0, 1.0, 1.0,  0.5, 1.0, 1.0, 1.0,
                1.0, 1.0, 0.5, 1.0,  1.0, 1.0, 0.5, 1.0,  1.0, 1.0, 0.5, 1.0,  1.0, 1.0, 0.5, 1.0,
            };
            _.vertex_buffers[0] = _.vertex_buffers[1] = make_buffer([&vertices](auto &_) {
                _.size = sizeof(vertices);
                _.content = vertices;
            });

            /* position components are at start of buffer */
            _.vertex_buffer_offsets[0] = 0;

            /* byte offset of color components in buffer */
            _.vertex_buffer_offsets[1] = 24 * 3 * sizeof(float);

            /* create an index buffer for the cube */
            uint16_t indices[] = {
                0, 1, 2,  0, 2, 3,
                6, 5, 4,  7, 6, 4,
                8, 9, 10,  8, 10, 11,
                14, 13, 12,  15, 14, 12,
                16, 17, 18,  16, 18, 19,
                22, 21, 20,  23, 22, 20
            };
            _.index_buffer = make_buffer([&indices](auto &_) {
                _.type = SG_BUFFERTYPE_INDEXBUFFER;
                _.size = sizeof(indices);
                _.content = indices;
            });
        });

        /*
            a pipeline object, note that we need to provide the
            MSAA sample count of the default framebuffer
        */
        _pipeline = make_pipeline([](auto &_) {
            _.shader = make_shader(noninterleaved_shader_desc());
            /* note how the vertex components are pulled from different buffer bind slots */
            /* positions come from vertex buffer slot 0 */
            _.layout.attrs[ATTR_vs_position].format = SG_VERTEXFORMAT_FLOAT3;
            _.layout.attrs[ATTR_vs_position].buffer_index = 0;
            /* colors come from vertex buffer slot 1 */
            _.layout.attrs[ATTR_vs_color0].format = SG_VERTEXFORMAT_FLOAT4;
            _.layout.attrs[ATTR_vs_color0].buffer_index = 1;
            _.index_type = SG_INDEXTYPE_UINT16;
            _.depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL;
            _.depth_stencil.depth_write_enabled = true;
            _.rasterizer.cull_mode = SG_CULLMODE_BACK;
        });

        /* default pass action */
        _pass_action = make<pass_action>();
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
