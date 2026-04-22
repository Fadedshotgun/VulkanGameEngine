#pragma once

#include "vCamera.hpp"
#include "vGameObject.hpp"

#include <vulkan/vulkan.h>

#define MAX_LIGHTS 10

struct PointLight {
    alignas(16) glm::vec4 position{}; // xyz = world position
    alignas(16) glm::vec4 color{};    // rgb = color, a = intensity
};

namespace v
{
    struct UniformBufferObject
    {
        glm::mat4 projection{1.f};
        glm::mat4 view{1.f};
        glm::mat4 inverseView{1.f};
        glm::vec4 ambientColor{1.f, 1.f, 1.f, 0.02f}; // r, g, b, intensity
        PointLight pointLights[MAX_LIGHTS];
        alignas(16) int numPointLights{0};
    };

    struct FrameInfo {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        vCamera &camera;
        VkDescriptorSet globalDescriptorSet;
        vGameObject::Map &gameObjects;
        int rasterMode;
    };
}