
#include "falcon.h"

namespace {

class app : public falcon::application {
    void init() override {
        _pass_action = falcon::gfx::make<falcon::gfx::pass_action>();
    }

    void frame() override {
        falcon::gfx::begin(_pass_action, width(), height());
    }

    falcon::gfx::pass_action _pass_action;
};

} // namespace

FALCON_MAIN(::app);
