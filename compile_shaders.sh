#!/usr/bin/env sh

./vulkan/macOS/bin/glslangValidator -V src/shaders/demo.vert
./vulkan/macOS/bin/glslangValidator -V src/shaders/demo.frag

mv vert.spv src/shaders
mv frag.spv src/shaders
