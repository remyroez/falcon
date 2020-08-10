
#include "falcon.h"

namespace {

class app : public falcon::application {
    // no impl
};

} // namespace

FALCON_MAIN(::app);
