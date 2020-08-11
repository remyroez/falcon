#ifndef FALCON_RENDERER_H_
#define FALCON_RENDERER_H_

#include "sokol_gfx.h"

namespace falcon {

// sokol_gfx rendering state class
class renderer {
public:
    // ctor
    renderer() : _pass_action{} {}

    // dtor
    virtual ~renderer() {}

    // begin default pass
    void begin_default_pass(int width, int height) {
        sg_begin_default_pass(&_pass_action, width, height);
    }

    // begin pass
    void begin_pass(sg_pass &pass) {
        sg_begin_pass(pass, &_pass_action);
    }

    // end pass
    void end_pass() {
        sg_end_pass();
    }

    // get color attachment action colors
    const auto &color_attachment_action_colors(int index) const { return color_attachment_action(index).val; }

public:
    // set clear color action
    void set_clear_color_action(int index, float r, float g, float b, float a = 1.f) {
        set_color_attachment_action(index, SG_ACTION_CLEAR, r, g, b, a);
    }

protected:
    // get color attachment action
    sg_color_attachment_action &color_attachment_action(int index) { return _pass_action.colors[index]; }
    const sg_color_attachment_action &color_attachment_action(int index) const { return _pass_action.colors[index]; }

    // set color attachment action
    void set_color_attachment_action(int index, sg_action action, float r, float g, float b, float a) {
        auto &color = color_attachment_action(index);
        color.action = action;
        color.val[0] = r;
        color.val[1] = g;
        color.val[2] = b;
        color.val[3] = a;
    }

    // get depth attachment action
    inline sg_depth_attachment_action &depth_attachment_action() { return _pass_action.depth; }

    // set depth attachment action
    void set_depth_attachment_action(sg_action action, float value) {
        auto &depth = depth_attachment_action();
        depth.action = action;
        depth.val = value;
    }

    // get stencil attachment action
    inline sg_stencil_attachment_action &stencil_attachment_action() { return _pass_action.stencil; }

    // set stencil attachment action
    void set_stencil_attachment_action(sg_action action, uint8_t value) {
        auto &stencil = stencil_attachment_action();
        stencil.action = action;
        stencil.val = value;
    }

protected:
    // pass action
    sg_pass_action _pass_action;
};

} // namespace falcon

#endif // FALCON_RENDERER_H_
