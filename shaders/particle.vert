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

layout (push_constant) uniform PushConstants {
    vec3 particlePosition;
    vec4 color;
    float size;
} pushConstants;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 projection;
    mat4 view;
    mat4 inverseView;
    vec4 ambientColor;
    PointLight pointLights[10];
    int numPointLights;
} ubo;

layout (location = 0) out vec2 fragOffset;

void main() {
    fragOffset = OFFSETS[gl_VertexIndex];

    vec4 particleInCameraSpace = ubo.view * vec4(pushConstants.particlePosition, 1.0);
    vec4 positionInCameraSpace = particleInCameraSpace + pushConstants.size * vec4(fragOffset, 0.0, 0.0);

    gl_Position = ubo.projection * positionInCameraSpace;
}