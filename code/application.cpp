#include "application.h"

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_args.h"
#include "sokol_time.h"
#include "sokol_fetch.h"
#include "sokol_glue.h"

namespace {

// init callback
void init(void *userdata) {
    if (auto *app = static_cast<falcon::application *>(userdata)) {
        app->init_cb();
    }
}

// frame callback
void frame(void *userdata) {
    if (auto *app = static_cast<falcon::application *>(userdata)) {
        app->frame_cb();
    }
}

// cleanup callback
void cleanup(void *userdata) {
    if (auto *app = static_cast<falcon::application *>(userdata)) {
        app->cleanup_cb();
    }
}

// event callback
void event(const sapp_event* ev, void *userdata) {
    if (auto *app = static_cast<falcon::application *>(userdata)) {
        app->event_cb(ev);
    }
}

// fail callback
void fail(const char *message, void *userdata) {
    if (auto *app = static_cast<falcon::application *>(userdata)) {
        app->fail_cb(message);
    }
}

} // namespace

namespace falcon {

void application::setup(int argc, char** argv, sapp_desc *desc) {
    // setup arguments
    {
        sargs_desc args{ argc, argv };
        sargs_setup(args);
    }

    // setup fetch
    {
        sfetch_desc_t desc{};
        //desc.max_requests = 1024;
        //desc.num_channels = 4;
        //desc.num_lanes = 8;
        sfetch_setup(desc);
    }

    // setup application
    configure_cb(desc);
}

void application::shutdown() {
    sfetch_shutdown();
    sargs_shutdown();
    sg_shutdown();
}

void application::quit() {
    sapp_request_quit();
}

void application::configure_cb(sapp_desc *desc) {
    // default config
    desc->user_data = this;
    desc->init_userdata_cb = ::init;
    desc->frame_userdata_cb = ::frame;
    desc->cleanup_userdata_cb = ::cleanup;
    desc->event_userdata_cb = ::event;
    desc->fail_userdata_cb = ::fail;
    desc->width = 1024;
    desc->height = 768;
    desc->gl_force_gles2 = true;
    desc->window_title = "falcon app";
    desc->ios_keyboard_resizes_canvas = false;

    // ユーザーコールバック
    configure(desc);
}

void application::init_cb() {
    // gfx
    sg_desc desc = {};
    desc.context = sapp_sgcontext();
    sg_setup(desc);

    // time
    stm_setup();

    // user callback
    init();
}

void application::frame_cb() {
    // update fetch
    sfetch_dowork();

    // update delta time
    _delta_time = stm_sec(stm_laptime(&_last_time));

    // user callback
    frame();

    // update gfx
    sg_commit();
}

void application::cleanup_cb() {
    // user callback
    cleanup();

    // shutdown application
    shutdown();
}

void application::event_cb(const sapp_event *ev) {
    // user callback
    event(ev);
}

void application::fail_cb(const char *message) {
    //SOKOL_LOG(message);
    
    // user callback
    fail(message);
}

} // namespace falcon
