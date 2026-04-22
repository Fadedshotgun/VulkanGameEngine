#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec3 fragPositionWorld;
layout (location = 2) out vec3 fragNormalWorld;
layout (location = 3) out vec2 fragTextCoord;

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

layout (push_constant) uniform Push {
    mat4 modelMatrix;
} push;

void main() {
    vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
    gl_Position = ubo.projection * ubo.view * positionWorld;

    fragNormalWorld = normalize(mat3(push.modelMatrix) * normal);
    fragPositionWorld = positionWorld.xyz;
    fragColor = color;
    fragTextCoord = uv;
}