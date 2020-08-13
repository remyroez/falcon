
#include "falcon.h"

namespace {

class app : public falcon::application {
    void configure(sapp_desc &desc) override {
        desc.width = 400;
        desc.height = 300;
        desc.window_title = "Clear (falcon app)";
    }

    void init() override {
        _pass_action = falcon::gfx::make<falcon::gfx::pass_action>([](auto &_) {
            _.colors[0] = falcon::gfx::make<falcon::gfx::color_attachment_action>([](auto &_) {
                _.action = SG_ACTION_CLEAR;
                _.val[0] = 1.f;
                _.val[1] = 0.f;
                _.val[2] = 0.f;
                _.val[3] = 1.f;
            });
        });
    }

    void frame() override {
        float g = _pass_action.colors[0].val[1] + 0.01f;
        _pass_action.colors[0].val[1] = (g > 1.0f) ? 0.0f : g;
        falcon::gfx::begin(_pass_action, width(), height());
    }

    falcon::gfx::pass_action _pass_action;
};

} // namespace

FALCON_MAIN(::app);
