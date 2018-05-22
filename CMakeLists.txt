cmake_minimum_required(VERSION 2.6)
project(wfn_eng)

set(CMAKE_CXX_FLAGS "-std=c++17 -g -Wall ${CMAKE_CXX_FLAGS}")

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

set(HEADERS
  src/vulkan.hpp
  src/error.hpp
  src/sdl.hpp
)

set(SOURCES
  src/vulkan/device.cpp
  src/vulkan/base.cpp
  src/vulkan/util.cpp

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
