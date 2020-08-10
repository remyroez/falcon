#ifndef FALCON_APPLICATION_H_
#define FALCON_APPLICATION_H_

#include "sokol_app.h"

namespace falcon {

// sokol_app wrapper
class application {
public:
    // ctor
    application() : _last_time(0), _delta_time(0.0) {}

    // dtor
    virtual ~application() {}

    // setup
    void setup(int argc, char** argv, sapp_desc *desc);

    // shutdown
    void shutdown();

    // quit
    void quit();

    // configure callback
    void configure_cb(sapp_desc *desc);

    // initialize callback
    void init_cb();

    // frame callback
    void frame_cb();

    // cleanup callback
    void cleanup_cb();

    // event callback
    void event_cb(const sapp_event *ev);

    // fail callback
    void fail_cb(const char *message);

    // get delta time
    inline double delta_time() const { return _delta_time; }

protected:
    // configure
    virtual void configure(sapp_desc *desc) {}

    // initialize
    virtual void init() {}

    // update
    virtual void frame() {}

    // cleanup
    virtual void cleanup() {}

    // event
    virtual void event(const sapp_event *ev) {}

    // fail
    virtual void fail(const char *message) {}

private:
    // last time
    uint64_t _last_time;

    // delta time
    double _delta_time;
};

} // namespace falcon

#define FALCON_MAIN(APP) \
sapp_desc sokol_main(int argc, char* argv[]) { \
    static APP _app; \
    sapp_desc desc = {}; \
    _app.setup(argc, argv, &desc); \
    return desc; \
}

#endif // FALCON_APPLICATION_H_
