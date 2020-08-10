# vars
set(FALCON_PATH ${CMAKE_SOURCE_DIR}/../code)
set(FALCON_INCLUDE_DIR ${FALCON_PATH})

# library: falcon
add_library(falcon STATIC)
target_include_directories(falcon PUBLIC ${FALCON_INCLUDE_DIR})
target_sources(falcon PRIVATE ${FALCON_PATH}/application.cpp)

# link sokol
include(cmake/sokol.cmake)
target_link_libraries(falcon PUBLIC ${SOKOL_LIBRARIES})

# vars
set(FALCON_LIBRARIES falcon)
