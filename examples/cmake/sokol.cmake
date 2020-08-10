# vars
set(SOKOL_PATH ${CMAKE_SOURCE_DIR}/thirdparty/sokol)
set(SOKOL_INCLUDE_DIR ${SOKOL_PATH})

# library: sokol
add_library(sokol STATIC)
target_include_directories(sokol PUBLIC ${SOKOL_INCLUDE_DIR} ${X11_INCLUDE_DIR})
target_sources(sokol PRIVATE ${CMAKE_SOURCE_DIR}/sokol.c)

# link libraries
find_package(X11 REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ALSA REQUIRED)
target_link_libraries(sokol PUBLIC
    ${X11_LIBRARIES}
    ${X11_Xcursor_LIB}
    ${X11_Xinput_LIB}
    Threads::Threads
    OpenGL::OpenGL
    ${ALSA_LIBRARIES}
    ${CMAKE_DL_LIBS}
    ${CMAKE_M_LIBS}
)

# TODO: renderer
target_compile_definitions(sokol PUBLIC SOKOL_GLCORE33)

# vars
set(SOKOL_LIBRARIES sokol)
