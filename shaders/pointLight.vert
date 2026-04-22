#version 450

const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

layout (location = 0) out vec2 fragOffset;

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

layout (push_constant) uniform PushConstants {
    vec3 lightPosition;
    vec4 color;
    float radius;
} pushConstants;

void main() {
    fragOffset = OFFSETS[gl_VertexIndex];
    // vec3 cameraRightWorld = vec3(ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]);
    // vec3 cameraUpWorld = vec3(ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]);

    // vec3 positionWorld = ubo.lightPosition.xyz
    // + LIGHT_RADIUS * fragOffset.x * cameraRightWorld
    // + LIGHT_RADIUS * fragOffset.y * cameraUpWorld;

    vec4 lightInCameraSpace = ubo.view * vec4(pushConstants.lightPosition, 1.0);
    vec4 positionInCameraSpace = lightInCameraSpace + pushConstants.radius * vec4(fragOffset, 0.0, 0.0);

    gl_Position = ubo.projection * positionInCameraSpace;
}