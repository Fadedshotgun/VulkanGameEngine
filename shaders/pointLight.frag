#version 450

layout (location = 0) in vec2 fragOffset;

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

layout (push_constant) uniform PushConstants {
    vec3 lightPosition;
    vec4 color;
    float radius;
} pushConstants;

const float PI = 3.1415926538;
void main() {
    float dis = sqrt(dot(fragOffset, fragOffset));
    if (dis > 1.0) {
        discard;
    }

    float cosDist = .5 * (cos(dis * PI) + 1);
    outColor = vec4(pushConstants.color.xyz + (cosDist/2), cosDist);
}