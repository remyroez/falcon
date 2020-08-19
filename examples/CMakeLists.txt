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
option(BUILD_EXAMPLE_ALL "Build all example" ON)
option(BUILD_EXAMPLE_MINIMUM "Build minimum example" OFF)
option(BUILD_EXAMPLE_CLEAR "Build clear example" OFF)
option(BUILD_EXAMPLE_TRIANGLE "Build triangle example" OFF)
option(BUILD_EXAMPLE_QUAD "Build quad example" OFF)
option(BUILD_EXAMPLE_BUFFEROFFSETS "Build bufferoffsets example" OFF)
option(BUILD_EXAMPLE_CUBE "Build cube example" OFF)
option(BUILD_EXAMPLE_NONINTERLEAVED "Build noninterleaved example" OFF)
option(BUILD_EXAMPLE_TEXCUBE "Build texcube example" OFF)
option(BUILD_EXAMPLE_OFFSCREEN "Build offscreen example" OFF)
option(BUILD_EXAMPLE_INSTANCING "Build instancing example" OFF)

# macro: add example executable
macro(add_example target_name)
    add_executable(${target_name} WIN32 MACOSX_BUNDLE)
    target_link_libraries(${target_name} ${FALCON_LIBRARIES})
    target_sources(${target_name} PRIVATE ${target_name}.cpp)
    target_compile_features(${target_name} PRIVATE cxx_std_17)
endmacro()

# macro: add example shader
macro(add_shader target_name slang)
    add_sokol_shader(
        shader_${target_name}
        ${SOKOL_SAMPLES_SAPP_PATH}/${target_name}-sapp.glsl
        ${CMAKE_CURRENT_SOURCE_DIR}/${target_name}-sapp.glsl.h
        ${slang}
    )
    add_dependencies(${target_name} shader_${target_name})
endmacro()

# macro: add example with shader
macro(add_example_with_shader target_name)
    add_example(${target_name})
    add_shader(${target_name} glsl330)
endmacro()

# example: minimum
if(BUILD_EXAMPLE_MINIMUM OR BUILD_EXAMPLE_ALL)
    add_example(minimum)
endif()

# example: clear
if(BUILD_EXAMPLE_CLEAR OR BUILD_EXAMPLE_ALL)
    add_example(clear)
endif()

# example: triangle
if(BUILD_EXAMPLE_TRIANGLE OR BUILD_EXAMPLE_ALL)
    add_example_with_shader(triangle)
endif()

# example: quad
if(BUILD_EXAMPLE_QUAD OR BUILD_EXAMPLE_ALL)
    add_example_with_shader(quad)
endif()

# example: bufferoffsets
if(BUILD_EXAMPLE_BUFFEROFFSETS OR BUILD_EXAMPLE_ALL)
    add_example_with_shader(bufferoffsets)
endif()

# example: cube
if(BUILD_EXAMPLE_CUBE OR BUILD_EXAMPLE_ALL)
    add_example_with_shader(cube)
    target_include_directories(cube PRIVATE ${HANDMADEMATH_INCLUDE_DIR})
endif()

# example: noninterleaved
if(BUILD_EXAMPLE_NONINTERLEAVED OR BUILD_EXAMPLE_ALL)
    add_example_with_shader(noninterleaved)
    target_include_directories(noninterleaved PRIVATE ${HANDMADEMATH_INCLUDE_DIR})
endif()

# example: texcube
if(BUILD_EXAMPLE_TEXCUBE OR BUILD_EXAMPLE_ALL)
    add_example_with_shader(texcube)
    target_include_directories(texcube PRIVATE ${HANDMADEMATH_INCLUDE_DIR})
endif()

# example: offscreen
if(BUILD_EXAMPLE_OFFSCREEN OR BUILD_EXAMPLE_ALL)
    add_example_with_shader(offscreen)
    target_include_directories(offscreen PRIVATE ${HANDMADEMATH_INCLUDE_DIR})
endif()

# example: instancing
if(BUILD_EXAMPLE_INSTANCING OR BUILD_EXAMPLE_ALL)
    add_example_with_shader(instancing)
    target_include_directories(instancing PRIVATE ${HANDMADEMATH_INCLUDE_DIR})
endif()
