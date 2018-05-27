cmake_minimum_required(VERSION 2.6)
project(wfn_eng)

set(CMAKE_CXX_FLAGS "-std=c++17 -g -Wall ${CMAKE_CXX_FLAGS}")

find_package(pkgconfig REQUIRED)

pkg_search_module(GLM glm REQUIRED)
pkg_search_module(SDL2 sdl2 REQUIRED)

link_directories(
    "/usr/local/lib"
    "${CMAKE_SOURCE_DIR}/vulkan/macOS/lib"
)

include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    "${CMAKE_SOURCE_DIR}/vulkan/macOS/include"
)

set(HEADERS
  src/vulkan/util.hpp
  src/vulkan.hpp
  src/error.hpp
  src/sdl.hpp
)

set(SOURCES
  src/vulkan/util/queue_family_indices.cpp
  src/vulkan/util/swapchain_support.cpp
  src/vulkan/util/pipeline.cpp
  src/vulkan/util/buffer.cpp
  src/vulkan/util/shader.cpp

  src/vulkan/command_pools.cpp
  src/vulkan/swapchain.cpp
  src/vulkan/device.cpp
  src/vulkan/base.cpp
  src/vulkan/core.cpp

  src/sdl/window.cpp

  src/error.cpp
  src/main.cpp
)

add_executable(wfn_eng ${SOURCES} ${HEADERS})

target_link_libraries(
    wfn_eng
    ${SDL2_LIBRARIES}
    MoltenVK
    vulkan
)

add_custom_command(TARGET ${PROJECT_NAME}
    PRE_BUILD
    COMMAND ./compile_shaders.sh
)
