
#include "falcon.h"

#define USE_SHADER
#ifdef USE_SHADER
#include "triangle-sapp.glsl.h"
#endif

namespace {

class app : public falcon::application {
public:
    app() : pip{}, bind{} {}

    void configure(sapp_desc *desc) override {
        desc->width = 640;
        desc->height = 480;
        desc->window_title = "Triangle (falcon app)";
    }

    void init() override {
        /* a vertex buffer with 3 vertices */
        float vertices[] = {
            // positions            // colors
             0.0f,  0.5f, 0.5f,     1.0f, 0.0f, 0.0f, 1.0f,
             0.5f, -0.5f, 0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f
        };
        bind.vertex_buffers[0] = falcon::gfx::make_buffer([&vertices](auto &_) {
            _.size = sizeof(vertices);
            _.content = vertices;
            _.label = "triangle-vertices";
        });

        /* create shader from code-generated sg_shader_desc */
#ifdef USE_SHADER
        sg_shader shd = falcon::gfx::make_shader(triangle_shader_desc());
#else
        auto shd = falcon::gfx::make_shader([](auto &_) {
            _.vs.source =
                "#version 330\n"
                "layout(location=0) in vec4 position;\n"
                "layout(location=1) in vec4 color0;\n"
                "out vec4 color;\n"
                "void main() {\n"
                "  gl_Position = position;\n"
                "  color = color0;\n"
                "}\n";
            _.fs.source =
                "#version 330\n"
                "in vec4 color;\n"
                "out vec4 frag_color;\n"
                "void main() {\n"
                "  frag_color = color;\n"
                "}\n";
        });
#endif

        /* create a pipeline object (default render states are fine for triangle) */
        pip = falcon::gfx::make_pipeline([&shd](auto &_) {
            _.shader = shd;
            /* if the vertex layout doesn't have gaps, don't need to provide strides and offsets */
            {
#ifndef USE_SHADER
                constexpr auto ATTR_vs_position = 0;
                constexpr auto ATTR_vs_color0 = 1;
#endif
                _.layout.attrs[ATTR_vs_position].format = SG_VERTEXFORMAT_FLOAT3;
                _.layout.attrs[ATTR_vs_color0].format = SG_VERTEXFORMAT_FLOAT4;
            }
            _.label = "triangle-pipeline";
        });

        _renderer.set_clear_color_action(0, 0.f, 0.f, 0.f);
    }

    void frame() override {
        _renderer.begin_default_pass(width(), height());
        sg_apply_pipeline(pip);
        sg_apply_bindings(bind);
        sg_draw(0, 3, 1);
        _renderer.end_pass();
    }

    falcon::gfx::pipeline pip;
    falcon::gfx::bindings bind;
    falcon::renderer _renderer;
};

} // namespace

FALCON_MAIN(::app);
