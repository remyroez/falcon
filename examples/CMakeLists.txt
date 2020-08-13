# setup CMake
cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0076 NEW)

# project: falcon examples
project(falcon_examples C CXX)

# includes
include(cmake/falcon.cmake)
include(cmake/sokol-samples.cmake)
include(cmake/sokol-tools-bin.cmake)

# options
option(BUILD_EXAMPLE_MINIMUM "Build minimum example" ON)
option(BUILD_EXAMPLE_CLEAR "Build clear example" ON)
option(BUILD_EXAMPLE_TRIANGLE "Build triangle example" ON)
option(BUILD_EXAMPLE_QUAD "Build quad example" ON)

# macro: add example executable
macro(add_example target_name)
    add_executable(${target_name} WIN32 MACOSX_BUNDLE)
    target_link_libraries(${target_name} ${FALCON_LIBRARIES})
    target_sources(${target_name} PRIVATE ${target_name}.cpp)
    target_compile_features(${target_name} PRIVATE cxx_std_17)
endmacro()

# macro: add example shader
macro(add_shader target_name input output slang)
    add_sokol_shader(shader_${target_name} ${input} ${CMAKE_CURRENT_SOURCE_DIR}/${output} ${slang})
    add_dependencies(${target_name} shader_${target_name})
endmacro()

# example: minimum
if(BUILD_EXAMPLE_MINIMUM)
    add_example(minimum)
endif()

# example: clear
if(BUILD_EXAMPLE_CLEAR)
    add_example(clear)
endif()

# example: triangle
if(BUILD_EXAMPLE_TRIANGLE)
    add_example(triangle)
    add_shader(triangle ${SOKOL_SAMPLES_SAPP_PATH}/triangle-sapp.glsl triangle-sapp.glsl.h glsl330)
endif()

# example: quad
if(BUILD_EXAMPLE_QUAD)
    add_example(quad)
    add_shader(quad ${SOKOL_SAMPLES_SAPP_PATH}/quad-sapp.glsl quad-sapp.glsl.h glsl330)
endif()
