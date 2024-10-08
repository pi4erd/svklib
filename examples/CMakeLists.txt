include_directories(../include)

add_executable(triangle triangle/triangle.cpp)
target_link_libraries(triangle svk)

add_executable(window window/window.cpp)
target_link_libraries(window svk)
