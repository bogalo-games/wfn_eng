#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexPos;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragTexPos) + 0.5 * vec4(fragColor, 0.0f);
}
