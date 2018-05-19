cmake_minimum_required(VERSION 2.6)
project(wfn_eng)

set(CMAKE_CXX_FLAGS "-std=c++17 ${CMAKE_CXX_FLAGS}")

link_directories(
    "/usr/local/lib"
    "${CMAKE_SOURCE_DIR}/vulkan/macOS/lib"
)

include_directories(
    ${GLM_INCLUDE_DIRS}
    "/usr/local/Cellar/glm/0.9.8.4/include"
    "/usr/local/Cellar/glfw/HEAD-fdfb5ab/include"
    "${CMAKE_SOURCE_DIR}/vulkan/macOS/include"
)

add_executable(wfn_eng src/main.cpp)

target_link_libraries(
    wfn_eng
    glfw
    MoltenVK
    vulkan
)
