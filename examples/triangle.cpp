
#include "falcon.h"
#include "triangle-sapp.glsl.h"

namespace {

class app : public falcon::application {
    void configure(sapp_desc *desc) override {
        desc->width = 640;
        desc->height = 480;
        desc->window_title = "Triangle (falcon app)";
    }

    void init() override {
        using namespace falcon::gfx;

        _bindings = make<bindings>([](auto &_) {
            /* a vertex buffer with 3 vertices */
            float vertices[] = {
                // positions            // colors
                 0.0f,  0.5f, 0.5f,     1.0f, 0.0f, 0.0f, 1.0f,
                 0.5f, -0.5f, 0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
                -0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f
            };
            _.vertex_buffers[0] = falcon::gfx::make_buffer([&vertices](auto &_) {
                _.size = sizeof(vertices);
                _.content = vertices;
                _.label = "triangle-vertices";
            });
        });

        /* create a pipeline object (default render states are fine for triangle) */
        _pipeline = falcon::gfx::make_pipeline([](auto &_) {
            /* create shader from code-generated sg_shader_desc */
            _.shader = falcon::gfx::make_shader(triangle_shader_desc());
            /* if the vertex layout doesn't have gaps, don't need to provide strides and offsets */
            _.layout.attrs[ATTR_vs_position].format = SG_VERTEXFORMAT_FLOAT3;
            _.layout.attrs[ATTR_vs_color0].format = SG_VERTEXFORMAT_FLOAT4;
            _.label = "triangle-pipeline";
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
                .draw(0, 3, 1);
    }

    falcon::gfx::pass_action _pass_action;
    falcon::gfx::pipeline _pipeline;
    falcon::gfx::bindings _bindings;
};

} // namespace

FALCON_MAIN(::app);
