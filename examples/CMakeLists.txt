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

# example: minimum
if(BUILD_EXAMPLE_MINIMUM)
    add_executable(minimum WIN32 MACOSX_BUNDLE)
    target_link_libraries(minimum ${FALCON_LIBRARIES})
    target_sources(minimum PRIVATE minimum.cpp)
    target_compile_features(minimum PRIVATE cxx_std_17)
endif()

# example: clear
if(BUILD_EXAMPLE_CLEAR)
    add_executable(clear WIN32 MACOSX_BUNDLE)
    target_link_libraries(clear ${FALCON_LIBRARIES})
    target_sources(clear PRIVATE clear.cpp)
    target_compile_features(clear PRIVATE cxx_std_17)
endif()

# example: triangle
if(BUILD_EXAMPLE_TRIANGLE)
    add_executable(triangle WIN32 MACOSX_BUNDLE)
    target_link_libraries(triangle ${FALCON_LIBRARIES})
    target_sources(triangle PRIVATE triangle.cpp)
    target_compile_features(triangle PRIVATE cxx_std_17)
endif()
