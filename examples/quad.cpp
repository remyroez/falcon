
#include "falcon.h"
#include "quad-sapp.glsl.h"

namespace {

class app : public falcon::application {
public:
    app() : pip{}, bind{} {}

    void configure(sapp_desc *desc) override {
        desc->width = 800;
        desc->height = 600;
        desc->window_title = "Quad (falcon app)";
    }

    void init() override {
        /* a vertex buffer */
        const float vertices[] = {
            // positions            colors
            -0.5f,  0.5f, 0.5f,     1.0f, 0.0f, 0.0f, 1.0f,
             0.5f,  0.5f, 0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
             0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f,
            -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 0.0f, 1.0f,
        };
        bind.vertex_buffers[0] = falcon::gfx::make_buffer([&vertices](auto &_) {
            _.size = sizeof(vertices);
            _.content = vertices;
            _.label = "quad-vertices";
        });

        /* an index buffer with 2 triangles */
        const uint16_t indices[] = { 0, 1, 2,  0, 2, 3 };
        bind.index_buffer = falcon::gfx::make_buffer([&indices](auto &_) {
            _.type = SG_BUFFERTYPE_INDEXBUFFER;
            _.size = sizeof(indices);
            _.content = indices;
            _.label = "quad-indices";
        });

        /* a pipeline state object */
        pip = falcon::gfx::make_pipeline([](auto &_) {
            /* a shader (use separate shader sources here */
            _.shader = falcon::gfx::make_shader(quad_shader_desc());
            _.index_type = SG_INDEXTYPE_UINT16;
            _.layout.attrs[ATTR_vs_position].format = SG_VERTEXFORMAT_FLOAT3;
            _.layout.attrs[ATTR_vs_color0].format = SG_VERTEXFORMAT_FLOAT4;
            _.label = "quad-pipeline";
        });

        /* default pass action */
        _renderer.set_clear_color_action(0, 0.f, 0.f, 0.f);
    }

    void frame() override {
        _renderer.begin_default_pass(width(), height());
        sg_apply_pipeline(pip);
        sg_apply_bindings(bind);
        sg_draw(0, 6, 1);
        _renderer.end_pass();
    }

    falcon::gfx::pipeline pip;
    falcon::gfx::bindings bind;
    falcon::renderer _renderer;
};

} // namespace

FALCON_MAIN(::app);
