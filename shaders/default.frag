#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPositionWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragTextCoord;

layout(set = 1, binding = 0) uniform sampler2D textureSampler;

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

layout (push_constant) uniform Push {
    mat4 modelMatrix;
} push;


void main() {
    vec3 diffuseLight = ubo.ambientColor.xyz * ubo.ambientColor.w; 
    vec3 specularLight = vec3(0.0);
    vec3 surfaceNormal = normalize(fragNormalWorld);

    vec3 cameraPositionWorld = ubo.inverseView[3].xyz;
    vec3 viewDirection = normalize(cameraPositionWorld - fragPositionWorld);

    for (int i = 0; i < ubo.numPointLights; i++) {
        PointLight light = ubo.pointLights[i];

        // diffuse
        vec3 directionToLight = light.position.xyz - fragPositionWorld;
        float attenuation = 1.0 / dot(directionToLight, directionToLight); 

        directionToLight = normalize(directionToLight);

        float cosAngleIncidence = max(dot(surfaceNormal, directionToLight), 0.0);
        vec3 intensity = light.color.xyz * light.color.w * attenuation;

        diffuseLight += intensity * cosAngleIncidence;

        // specular
        vec3 halfAngle = normalize(directionToLight + viewDirection);
        float blinnTerm = dot(surfaceNormal, halfAngle);
        blinnTerm = clamp(blinnTerm, 0, 1);
        blinnTerm = pow(blinnTerm, 512); // higher power -> sharper highlight
        specularLight += intensity * blinnTerm;
    }

    vec4 textureColor = texture(textureSampler, fragTextCoord);
    outColor = vec4((diffuseLight * fragColor) + (specularLight * fragColor), 1.0) * textureColor;
}