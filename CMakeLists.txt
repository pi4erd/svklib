cmake_minimum_required(VERSION 3.20)

project(sgllib VERSION 0.1.0)

find_package(fmt REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

FILE(GLOB SVK_SOURCE_FILES core/*.cpp)

include_directories(include)

add_library(svk ${SVK_SOURCE_FILES})

set(SGL_LIBRARIES svk PARENT_SCOPE)

set(SGL_INCLUDE_DIRECTORIES ${CMAKE_PROJECT_SOURCE_DIR}/include PARENT_SCOPE)

target_link_libraries(svk
    PUBLIC fmt Vulkan::Vulkan glfw
)

add_subdirectory(examples)
