
#include "falcon.h"
#include "quad-sapp.glsl.h"

namespace {

class app : public falcon::application {
    void configure(sapp_desc &desc) override {
        desc.width = 800;
        desc.height = 600;
        desc.window_title = "Quad (falcon app)";
    }

    void init() override {
        using namespace falcon::gfx;

        _bindings = make<bindings>([](auto &_) {
            /* a vertex buffer */
            const float vertices[] = {
                // positions            colors
                -0.5f,  0.5f, 0.5f,     1.0f, 0.0f, 0.0f, 1.0f,
                 0.5f,  0.5f, 0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
                 0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f,
                -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 0.0f, 1.0f,
            };
            _.vertex_buffers[0] = make_buffer([&vertices](auto &_) {
                _.size = sizeof(vertices);
                _.content = vertices;
                _.label = "quad-vertices";
            });

            /* an index buffer with 2 triangles */
            const uint16_t indices[] = { 0, 1, 2,  0, 2, 3 };
            _.index_buffer = make_buffer([&indices](auto &_) {
                _.type = SG_BUFFERTYPE_INDEXBUFFER;
                _.size = sizeof(indices);
                _.content = indices;
                _.label = "quad-indices";
            });
        });

        /* a pipeline state object */
        _pipeline = make_pipeline([](auto &_) {
            /* a shader (use separate shader sources here */
            _.shader = make_shader(quad_shader_desc());
            _.index_type = SG_INDEXTYPE_UINT16;
            _.layout.attrs[ATTR_vs_position].format = SG_VERTEXFORMAT_FLOAT3;
            _.layout.attrs[ATTR_vs_color0].format = SG_VERTEXFORMAT_FLOAT4;
            _.label = "quad-pipeline";
        });

        /* default pass action */
        _pass_action = make<pass_action>([](auto &_) {
            _.colors[0] = make<color_attachment_action>([](auto &_) {
                _.action = SG_ACTION_CLEAR;
                _.val[0] = 0.f;
                _.val[1] = 0.f;
                _.val[2] = 0.f;
                _.val[3] = 1.f;
            });
        });
    }

    void frame() override {
        falcon::gfx::begin(_pass_action, width(), height())
            .pipeline(_pipeline)
                .bindings(_bindings)
                .draw(0, 6, 1);
    }

    falcon::gfx::pass_action _pass_action;
    falcon::gfx::pipeline _pipeline;
    falcon::gfx::bindings _bindings;
};

} // namespace

FALCON_MAIN(::app);
