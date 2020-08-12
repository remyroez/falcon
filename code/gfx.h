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
using bindings = sg_bindings;

// function wrappers
inline buffer make_buffer(const sg_buffer_desc& desc) { return sg_make_buffer(desc); }
inline buffer make_buffer(std::function<void(sg_buffer_desc&)> fn) {
    sg_buffer_desc desc{};
    fn(desc);
    return make_buffer(desc);
}

inline image make_image(const sg_image_desc& desc) { return sg_make_image(desc); }
inline image make_image(std::function<void(sg_image_desc&)> fn) {
    sg_image_desc desc{};
    fn(desc);
    return make_image(desc);
}

inline shader make_shader(const sg_shader_desc& desc) { return sg_make_shader(desc); }
inline shader make_shader(std::function<void(sg_shader_desc&)> fn) {
    sg_shader_desc desc{};
    fn(desc);
    return make_shader(desc);
}

inline pipeline make_pipeline(const sg_pipeline_desc& desc) { return sg_make_pipeline(desc); }
inline pipeline make_pipeline(std::function<void(sg_pipeline_desc&)> fn) {
    sg_pipeline_desc desc{};
    fn(desc);
    return make_pipeline(desc);
}

} // namespace falcon::gfx

#endif // FALCON_GFX_H_
