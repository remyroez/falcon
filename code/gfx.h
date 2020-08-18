#ifndef FALCON_GFX_H_
#define FALCON_GFX_H_

#include <functional>

#include "sokol_gfx.h"

namespace falcon::gfx {

// id aliases
using buffer = sg_buffer;
using image = sg_image;
using shader = sg_shader;
using pipeline = sg_pipeline;
using pass = sg_pass;
using context = sg_context;

// struct aliases
using pass_action = sg_pass_action;
using color_attachment_action = sg_color_attachment_action;
using bindings = sg_bindings;

// function wrappers
inline auto make_buffer(const sg_buffer_desc* desc) { return sg_make_buffer(desc); }
inline auto make_buffer(const sg_buffer_desc& desc) { return sg_make_buffer(desc); }
inline auto make_buffer(std::function<void(sg_buffer_desc&)> fn) {
    sg_buffer_desc desc{};
    fn(desc);
    return make_buffer(desc);
}
inline auto make_vertex_buffer(void *content, int size, const char *label = nullptr) {
    return make_buffer([&](auto &_) {
        _.type = SG_BUFFERTYPE_VERTEXBUFFER;
        _.size = size;
        _.content = content;
        _.label = label;
    });
}
inline auto make_index_buffer(void *content, int size, const char *label = nullptr) {
    return make_buffer([&](auto &_) {
        _.type = SG_BUFFERTYPE_INDEXBUFFER;
        _.size = size;
        _.content = content;
        _.label = label;
    });
}

inline auto make_image(const sg_image_desc* desc) { return sg_make_image(desc); }
inline auto make_image(const sg_image_desc& desc) { return sg_make_image(desc); }
inline auto make_image(std::function<void(sg_image_desc&)> fn) {
    sg_image_desc desc{};
    fn(desc);
    return make_image(desc);
}

inline auto make_shader(const sg_shader_desc* desc) { return sg_make_shader(desc); }
inline auto make_shader(const sg_shader_desc& desc) { return sg_make_shader(desc); }
inline auto make_shader(std::function<void(sg_shader_desc&)> fn) {
    sg_shader_desc desc{};
    fn(desc);
    return make_shader(desc);
}

inline auto make_pipeline(const sg_pipeline_desc* desc) { return sg_make_pipeline(desc); }
inline auto make_pipeline(const sg_pipeline_desc& desc) { return sg_make_pipeline(desc); }
inline auto make_pipeline(std::function<void(sg_pipeline_desc&)> fn) {
    sg_pipeline_desc desc{};
    fn(desc);
    return make_pipeline(desc);
}

inline auto make_pass(const sg_pass_desc* desc) { return sg_make_pass(desc); }
inline auto make_pass(const sg_pass_desc& desc) { return sg_make_pass(desc); }
inline auto make_pass(std::function<void(sg_pass_desc&)> fn) {
    sg_pass_desc desc{};
    fn(desc);
    return make_pass(desc);
}

template <class T>
inline T make(std::function<void(T&)> fn) {
    T instance{};
    fn(instance);
    return std::move(instance);
}

template <class T>
inline T make() {
    return T{};
}

inline auto make_pass_action_clear(float r, float g, float b, float a = 1.f) {
    return make<pass_action>([&](auto &_) {
        _.colors[0] = make<color_attachment_action>([&](auto &_) {
            _.action = SG_ACTION_CLEAR;
            _.val[0] = r;
            _.val[1] = g;
            _.val[2] = b;
            _.val[3] = a;
        });
    });
}

// pipeline state
struct pipeline_state final {
    inline operator bool() { return true; }

    inline pipeline_state &bindings(const sg_bindings* bindings) {
        sg_apply_bindings(bindings);
        return *this;
    }
    inline pipeline_state &bindings(const sg_bindings& bindings) {
        sg_apply_bindings(bindings);
        return *this;
    }

    inline pipeline_state &uniforms(sg_shader_stage stage, int ub_index, const void* data, int num_bytes) {
        sg_apply_uniforms(stage, ub_index, data, num_bytes);
        return *this;
    }

    inline pipeline_state &draw(int base_element, int num_elements, int num_instances) {
        sg_draw(base_element, num_elements, num_instances);
        return *this;
    }
};

// pass state
struct pass_state final {
    ~pass_state() { sg_end_pass(); }

    inline operator bool() { return true; }

    inline pass_state &viewport(int x, int y, int width, int height, bool origin_top_left) {
        sg_apply_viewport(x, y, width, height, origin_top_left);
        return *this;
    }

    inline pass_state &scissor_rect(int x, int y, int width, int height, bool origin_top_left) {
        sg_apply_scissor_rect(x, y, width, height, origin_top_left);
        return *this;
    }

    inline pass_state &uniforms(sg_shader_stage stage, int ub_index, const void* data, int num_bytes) {
        sg_apply_uniforms(stage, ub_index, data, num_bytes);
        return *this;
    }

    inline pipeline_state pipeline(pipeline &pip) {
        sg_apply_pipeline(pip);
        return pipeline_state{};
    }
};

inline pass_state begin(const sg_pass_action* pass_action, int width, int height) {
    sg_begin_default_pass(pass_action, width, height);
    return pass_state{};
}

inline pass_state begin(const sg_pass_action& pass_action, int width, int height) {
    return begin(&pass_action, width, height);
}

inline pass_state begin(sg_pass pass, const sg_pass_action* pass_action) {
    sg_begin_pass(pass, pass_action);
    return pass_state{};
}

inline pass_state begin(sg_pass pass, const sg_pass_action& pass_action) {
    return begin(pass, &pass_action);
}

} // namespace falcon::gfx

#endif // FALCON_GFX_H_
