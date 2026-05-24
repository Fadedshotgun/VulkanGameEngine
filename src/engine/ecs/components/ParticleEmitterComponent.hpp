#pragma once
#include "vDevice.hpp"
#include <glm/glm.hpp>
#include <vector>

namespace ecs
{
    struct InstanceData
    {
        VkBuffer buffer{VK_NULL_HANDLE};
        VkDeviceMemory memory{VK_NULL_HANDLE};
        void *mappedPointer{nullptr};
        size_t capacity{0};

        InstanceData() = default;

        // delete copy
        InstanceData(const InstanceData &) = delete;
        InstanceData &operator=(const InstanceData &) = delete;

        // move memory
        InstanceData(InstanceData &&other) noexcept : buffer(other.buffer), memory(other.memory), mappedPointer(other.mappedPointer), capacity(other.capacity)
        {
            other.buffer = VK_NULL_HANDLE;
            other.memory = VK_NULL_HANDLE;
            other.mappedPointer = nullptr;
            other.capacity = 0;
        }

        InstanceData &operator=(InstanceData &&other) noexcept
        {
            buffer = other.buffer;
            memory = other.memory;
            mappedPointer = other.mappedPointer;
            capacity = other.capacity;

            other.buffer = VK_NULL_HANDLE;
            other.memory = VK_NULL_HANDLE;
            other.mappedPointer = nullptr;
            other.capacity = 0;
            return *this;
        }
    };

    struct Particle
    {
        glm::vec3 position;
        glm::vec3 color;
        float speed;
        float size;
        glm::vec3 emissionDirection;

        float lifetime;
        float timePassed;
    };

    struct ParticleEmitterComponent
    {
        float emissionRate{0}; // particles per second
        float lifetime{0};
        float speed{0};
        float size{0};
        float angle1{0};
        glm::vec3 emissionDirection{0, 1, 0};
        glm::vec3 color{};

        // internal data
        std::vector<Particle> particles{};
        float timeSinceLastEmission{0};

        // GPU instancing
        InstanceData instanceData{};
    };
}