# setup CMake
cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0076 NEW)

# project: falcon examples
project(falcon_examples C CXX)

# include: falcon
include(cmake/falcon.cmake)

# options
option(BUILD_EXAMPLE_MINIMUM "Build minimum example" ON)
option(BUILD_EXAMPLE_CLEAR "Build clear example" ON)
option(BUILD_EXAMPLE_TRIANGLE "Build triangle example" ON)

# macro: add example executable
macro(add_example target_name)
    add_executable(${target_name} WIN32 MACOSX_BUNDLE)
    target_link_libraries(${target_name} ${FALCON_LIBRARIES})
    target_sources(${target_name} PRIVATE ${target_name}.cpp)
    target_compile_features(${target_name} PRIVATE cxx_std_17)
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
endif()
