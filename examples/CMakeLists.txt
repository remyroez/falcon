# setup CMake
cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0076 NEW)

# project: falcon examples
project(falcon_examples C CXX)

# include: falcon
include(cmake/falcon.cmake)

# options
option(BUILD_EXAMPLE_MINIMUM "Build minimum example" ON)

# example: minimum
if(BUILD_EXAMPLE_MINIMUM)
    add_executable(minimum WIN32 MACOSX_BUNDLE)
    target_link_libraries(minimum ${FALCON_LIBRARIES})
    target_sources(minimum PRIVATE minimum.cpp)
endif()
