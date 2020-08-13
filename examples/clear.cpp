
#include "falcon.h"

namespace {

class app : public falcon::application {
    void configure(sapp_desc &desc) override {
        desc.width = 400;
        desc.height = 300;
        desc.window_title = "Clear (falcon app)";
    }

    void init() override {
        _renderer.set_clear_color_action(0, 1.f, 0.f, 0.f);
    }

    void frame() override {
        const auto &colors = _renderer.color_attachment_action_colors(0);
        auto g = colors[1] + 0.01f;
        _renderer.set_clear_color_action(0, colors[0], (g > 1.0f) ? 0.f : g, colors[2], colors[3]);

        _renderer.begin_default_pass(width(), height());
        _renderer.end_pass();
    }

    falcon::renderer _renderer;
};

} // namespace

FALCON_MAIN(::app);
