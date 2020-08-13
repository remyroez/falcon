# vars
set(SOKOL_TOOLS_BIN_PATH ${CMAKE_SOURCE_DIR}/thirdparty/sokol-tools-bin)
set(SOKOL_SHDC_PATH ${SOKOL_TOOLS_BIN_PATH}/bin/linux/sokol-shdc)

# macro: add sokol shader
macro(add_sokol_shader target_name input output slang)
    add_custom_target(${target_name}
        SOURCES ${output}
    )
    add_custom_command(OUTPUT ${output}
        COMMAND ${SOKOL_SHDC_PATH} --input ${input} --output ${output} --slang ${slang}
    )
endmacro()
