#include <stdlib.h> /* rand() */

#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE
#include "HandmadeMath.h"

#include "falcon.h"
#include "instancing-sapp.glsl.h"

#define MAX_PARTICLES (512 * 1024)
#define NUM_PARTICLES_EMITTED_PER_FRAME (10)

namespace {

class app : public falcon::application {
    void configure(sapp_desc &desc) override {
        desc.width = 800;
        desc.height = 600;
        desc.sample_count = 4;
        desc.window_title = "Instancing (falcon app)";

        _ry = 0.f;
        _vs_params = vs_params_t{};
        _cur_num_particles = 0;
        for (auto &pos : _pos) {
            pos = {};
        }
        for (auto &vel : _vel) {
            vel = {};
        }
    }

    void init() override {
        using namespace falcon::gfx;

        /* a pass action for the default render pass */
        _pass_action = make_pass_action_clear(0.0f, 0.0f, 0.0f);

        _bindings = make<bindings>([](auto &_) {
            /* vertex buffer for static geometry, goes into vertex-buffer-slot 0 */
            const float r = 0.05f;
            const float vertices[] = {
                // positions            colors
                0.0f,   -r, 0.0f,       1.0f, 0.0f, 0.0f, 1.0f,
                   r, 0.0f, r,          0.0f, 1.0f, 0.0f, 1.0f,
                   r, 0.0f, -r,         0.0f, 0.0f, 1.0f, 1.0f,
                  -r, 0.0f, -r,         1.0f, 1.0f, 0.0f, 1.0f,
                  -r, 0.0f, r,          0.0f, 1.0f, 1.0f, 1.0f,
                0.0f,    r, 0.0f,       1.0f, 0.0f, 1.0f, 1.0f
            };
            _.vertex_buffers[0] = make_vertex_buffer(vertices, sizeof(vertices), "geometry-vertices");

            /* index buffer for static geometry */
            const uint16_t indices[] = {
                0, 1, 2,    0, 2, 3,    0, 3, 4,    0, 4, 1,
                5, 1, 2,    5, 2, 3,    5, 3, 4,    5, 4, 1
            };
            _.index_buffer = make_index_buffer(indices, sizeof(indices), "geometry-indices");

            /* empty, dynamic instance-data vertex buffer, goes into vertex-buffer-slot 1 */
            _.vertex_buffers[1] = make_buffer([](auto &_) {
                _.size = MAX_PARTICLES * sizeof(hmm_vec3);
                _.usage = SG_USAGE_STREAM;
                _.label = "instance-data";
            });
        });

        /* a pipeline object */
        _pipeline = make_pipeline([](auto &_) {
            /* a shader */
            _.shader = make_shader(instancing_shader_desc());

            /* vertex buffer at slot 1 must step per instance */
            _.layout.buffers[1].step_func = SG_VERTEXSTEP_PER_INSTANCE;
            _.layout.attrs[ATTR_vs_pos].format = SG_VERTEXFORMAT_FLOAT3;
            _.layout.attrs[ATTR_vs_pos].buffer_index = 0;
            _.layout.attrs[ATTR_vs_color0].format = SG_VERTEXFORMAT_FLOAT4;
            _.layout.attrs[ATTR_vs_color0].buffer_index = 0;
            _.layout.attrs[ATTR_vs_inst_pos].format = SG_VERTEXFORMAT_FLOAT3;
            _.layout.attrs[ATTR_vs_inst_pos].buffer_index = 1;

            _.index_type = SG_INDEXTYPE_UINT16;
            _.depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL;
            _.depth_stencil.depth_write_enabled = true;
            _.rasterizer.cull_mode = SG_CULLMODE_BACK;
            _.label = "instancing-pipeline";
        });
    }

    void frame() override {
        const float w = (float)width(), h = (float)height();
        const float frame_time = delta_time();

        /* emit new particles */
        for (int i = 0; i < NUM_PARTICLES_EMITTED_PER_FRAME; i++) {
            if (_cur_num_particles < MAX_PARTICLES) {
                _pos[_cur_num_particles] = HMM_Vec3(0.0, 0.0, 0.0);
                _vel[_cur_num_particles] = HMM_Vec3(
                    ((float)(rand() & 0x7FFF) / 0x7FFF) - 0.5f,
                    ((float)(rand() & 0x7FFF) / 0x7FFF) * 0.5f + 2.0f,
                    ((float)(rand() & 0x7FFF) / 0x7FFF) - 0.5f);
                _cur_num_particles++;
            }
            else {
                break;
            }
        }

        /* update particle positions */
        for (int i = 0; i < _cur_num_particles; i++) {
            _vel[i].Y -= 1.0f * frame_time;
            _pos[i].X += _vel[i].X * frame_time;
            _pos[i].Y += _vel[i].Y * frame_time;
            _pos[i].Z += _vel[i].Z * frame_time;
            /* bounce back from 'ground' */
            if (_pos[i].Y < -2.0f) {
                _pos[i].Y = -1.8f;
                _vel[i].Y = -_vel[i].Y;
                _vel[i].X *= 0.8f; _vel[i].Y *= 0.8f; _vel[i].Z *= 0.8f;
            }
        }

        /* update instance data */
        falcon::gfx::update_buffer(_bindings.vertex_buffers[1], _pos, _cur_num_particles*sizeof(hmm_vec3));

        /* model-view-projection matrix */
        hmm_mat4 proj = HMM_Perspective(60.0f, w/h, 0.01f, 50.0f);
        hmm_mat4 view = HMM_LookAt(HMM_Vec3(0.0f, 1.5f, 12.0f), HMM_Vec3(0.0f, 0.0f, 0.0f), HMM_Vec3(0.0f, 1.0f, 0.0f));
        hmm_mat4 view_proj = HMM_MultiplyMat4(proj, view);
        _ry += 1.0f;
        _vs_params.mvp = HMM_MultiplyMat4(view_proj, HMM_Rotate(_ry, HMM_Vec3(0.0f, 1.0f, 0.0f)));;

        /* ...and draw */
        falcon::gfx::begin(_pass_action, w, h)
            .pipeline(_pipeline)
                .bindings(_bindings)
                .uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &_vs_params, sizeof(_vs_params))
                .draw(0, 24, _cur_num_particles);
    }

    vs_params_t _vs_params;
    float _ry;
    falcon::gfx::pass_action _pass_action;
    falcon::gfx::pipeline _pipeline;
    falcon::gfx::bindings _bindings;
    int _cur_num_particles;
    hmm_vec3 _pos[MAX_PARTICLES];
    hmm_vec3 _vel[MAX_PARTICLES];
};

} // namespace

FALCON_MAIN(::app);
