include_directories(../include)

add_executable(triangle triangle/triangle.cpp)
target_link_libraries(triangle svk)

add_executable(window window/window.cpp)
target_link_libraries(window svk)

file(GLOB
    SHADER_SOURCES
    shaders/*.vert
    shaders/*.frag
    shaders/*.comp
)

find_package(Vulkan REQUIRED)

foreach(SHADER IN LISTS SHADER_SOURCES)
    get_filename_component(FILENAME ${SHADER} NAME)
    add_custom_command(OUTPUT shaders/${FILENAME}.spv
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o shaders/${FILENAME}.spv
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${FILENAME}"
    )
    list(APPEND SPV_SHADERS shaders/${FILENAME}.spv)
endforeach()

add_custom_target(compile_shaders ALL DEPENDS ${SPV_SHADERS})
add_dependencies(triangle compile_shaders)
