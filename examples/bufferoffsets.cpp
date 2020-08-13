#include "falcon.h"
#include "bufferoffsets-sapp.glsl.h"

namespace {

class app : public falcon::application {
    void configure(sapp_desc &desc) override {
        desc.width = 800;
        desc.height = 600;
        desc.window_title = "Buffer Offsets (falcon app)";
    }

    typedef struct {
        float x, y, r, g, b;
    } vertex_t;

    using index_t = uint16_t;

    void init() override {
        using namespace falcon::gfx;

        _bindings = make<bindings>([](auto &_) {
            /* a 2D triangle and quad in 1 vertex buffer and 1 index buffer */
            vertex_t vertices[7] = {
                /* triangle */
                {  0.0f,   0.55f,  1.0f, 0.0f, 0.0f },
                {  0.25f,  0.05f,  0.0f, 1.0f, 0.0f },
                { -0.25f,  0.05f,  0.0f, 0.0f, 1.0f },

                /* quad */
                { -0.25f, -0.05f,  0.0f, 0.0f, 1.0f },
                {  0.25f, -0.05f,  0.0f, 1.0f, 0.0f },
                {  0.25f, -0.55f,  1.0f, 0.0f, 0.0f },
                { -0.25f, -0.55f,  1.0f, 1.0f, 0.0f }
            };
            index_t indices[9] = {
                0, 1, 2,
                0, 1, 2, 0, 2, 3
            };
            _.vertex_buffers[0] = make_buffer([&vertices](auto &_) {
                _.size = sizeof(vertices);
                _.content = vertices;
            });
            _.index_buffer = make_buffer([&indices](auto &_) {
                _.type = SG_BUFFERTYPE_INDEXBUFFER;
                _.size = sizeof(indices);
                _.content = indices;
            });
        });

        /* a shader and pipeline to render 2D shapes */
        _pipeline = make_pipeline([](auto &_) {
            _.shader = make_shader(bufferoffsets_shader_desc());
            _.index_type = SG_INDEXTYPE_UINT16;
            _.layout.attrs[ATTR_vs_position].format = SG_VERTEXFORMAT_FLOAT2;
            _.layout.attrs[ATTR_vs_color0].format = SG_VERTEXFORMAT_FLOAT3;
        });

        /* default pass action */
        _pass_action = make<pass_action>([](auto &_) {
            _.colors[0] = make<color_attachment_action>([](auto &_) {
                _.action = SG_ACTION_CLEAR;
                _.val[0] = 0.5f;
                _.val[1] = 0.5f;
                _.val[2] = 1.f;
                _.val[3] = 1.f;
            });
        });
    }

    void frame() override {
        if (auto pass = falcon::gfx::begin(_pass_action, width(), height())) {
            if (auto pip = pass.pipeline(_pipeline)) {
                /* render the triangle */
                _bindings.vertex_buffer_offsets[0] = 0;
                _bindings.index_buffer_offset = 0;
                pip.bindings(_bindings).draw(0, 3, 1);

                /* render the quad */
                _bindings.vertex_buffer_offsets[0] = 3 * sizeof(vertex_t);
                _bindings.index_buffer_offset = 3 * sizeof(index_t);
                pip.bindings(_bindings).draw(0, 6, 1);
            }
        }
    }

    falcon::gfx::pass_action _pass_action;
    falcon::gfx::pipeline _pipeline;
    falcon::gfx::bindings _bindings;
};

} // namespace

FALCON_MAIN(::app);
