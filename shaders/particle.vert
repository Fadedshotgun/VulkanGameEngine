#version 450

const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

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

layout (location = 0) out vec2 fragOffset;
layout (location = 1) out vec4 fragColor;

layout (location = 0) in vec3 position;
layout (location = 1) in float size;
layout (location = 2) in vec4 color;


void main() {
    fragOffset = OFFSETS[gl_VertexIndex] * size;

    vec4 particleInCameraSpace = ubo.view * vec4(position, 1.0);
    vec4 positionInCameraSpace = particleInCameraSpace + vec4(fragOffset, 0.0, 0.0);

    gl_Position = ubo.projection * positionInCameraSpace;
    fragColor = color;
}