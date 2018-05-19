cmake_minimum_required(VERSION 2.6)
project(wfn_eng)

set(CMAKE_CXX_FLAGS "-std=c++17 ${CMAKE_CXX_FLAGS}")

find_package(pkgconfig REQUIRED)

pkg_search_module(glm glm)
pkg_search_module(SDL2 sdl2)

link_directories(
    "/usr/local/lib"
    "${CMAKE_SOURCE_DIR}/vulkan/macOS/lib"
)

include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    "${CMAKE_SOURCE_DIR}/vulkan/macOS/include"
)

add_executable(wfn_eng src/main.cpp)

target_link_libraries(
    wfn_eng
    ${SDL2_LIBRARIES}
    MoltenVK
    vulkan
)
