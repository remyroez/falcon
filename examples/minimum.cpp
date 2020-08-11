
#include "falcon.h"

namespace {

class app : public falcon::application {
    void frame() override {
        _renderer.begin_default_pass(width(), height());
        _renderer.end_pass();
    }

    falcon::renderer _renderer;
};

} // namespace

FALCON_MAIN(::app);
