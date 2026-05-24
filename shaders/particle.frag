#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 1) in vec4 fragColor;

layout (location = 0) out vec4 outColor;

struct PointLight {
    vec4 position;
    vec4 color;
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 projection;
    mat4 view;
    mat4 inverseView;
    vec4 ambientColor;
    PointLight pointLights[10];
    int numPointLights;
} ubo;


void main() {
    outColor = fragColor;
}