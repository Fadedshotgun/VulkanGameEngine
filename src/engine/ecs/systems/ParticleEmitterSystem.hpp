#pragma once

#include "Components.hpp"
#include "EntityRegistry.hpp"
#include "vDevice.hpp"

#include <algorithm>
#include <random>

namespace ecs
{
    // stole this lol
    static glm::vec3 randomSpreadDirection(glm::vec3 baseDirection, float spread)
    {
        if (spread == 0.f)
            return baseDirection;

        static std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> angleDist(0.f, spread);
        std::uniform_real_distribution<float> rotDist(0.f, glm::two_pi<float>());

        float angle = angleDist(rng);
        float azimuth = rotDist(rng);

        glm::vec3 up = glm::abs(baseDirection.y) < 0.9f ? glm::vec3(0, 1, 0) : glm::vec3(0, 0, 1);
        glm::vec3 right = glm::normalize(glm::cross(up, baseDirection));
        glm::vec3 forward = glm::normalize(glm::cross(baseDirection, right));

        float sinAngle = glm::sin(angle);
        return glm::normalize(
            baseDirection * glm::cos(angle) +
            right * sinAngle * glm::cos(azimuth) +
            forward * sinAngle * glm::sin(azimuth));
    }

    inline void emitParticle(ParticleEmitterComponent &emitter, TransformComponent &transform, int count = 1)
    {
        for (int i = 0; i < count; i++)
        {
            glm::vec3 dir = randomSpreadDirection(emitter.emissionDirection, glm::radians(emitter.angle1));
            Particle newParticle = {
                transform.translation,
                emitter.color,
                emitter.speed / 10,
                emitter.size,
                dir,
                emitter.lifetime,
                0};
            emitter.particles.push_back(newParticle);
        }
    }

    inline void clampEmitter(ParticleEmitterComponent &emitter)
    {
        emitter.size = std::max(0.f, emitter.size);
        emitter.emissionRate = std::max(0.f, emitter.emissionRate);
        emitter.lifetime = std::max(0.f, emitter.lifetime);
        emitter.angle1 = std::max(0.f, emitter.angle1);
    }

    inline void updateParticleEmitter(ParticleEmitterComponent &emitter, TransformComponent &transform, float frameTime)
    {
        clampEmitter(emitter);

        if (emitter.size == 0 || emitter.emissionRate == 0 || emitter.lifetime == 0)
        {
            return;
        }

        emitter.timeSinceLastEmission += frameTime;
        float particlesPerSecond = 1 / emitter.emissionRate;

        if (emitter.timeSinceLastEmission > particlesPerSecond)
        {
            int numberOfParticles = floor(emitter.timeSinceLastEmission / particlesPerSecond);

            emitter.timeSinceLastEmission = fmod(emitter.timeSinceLastEmission, particlesPerSecond);
            emitParticle(emitter, transform, numberOfParticles);
        }

        for (auto i = emitter.particles.begin(); i != emitter.particles.end();)
        {
            i->position += i->emissionDirection * i->speed * frameTime;
            i->timePassed += frameTime;

            if (i->timePassed > i->lifetime)
            {
                *i = emitter.particles.back();
                emitter.particles.pop_back();
            }
            else
            {
                ++i;
            }
        }
    }

    inline void destroyParticleEmitterInstanceData(ParticleEmitterComponent &emitter, v::vDevice &device)
    {
        if (emitter.instanceData.buffer == VK_NULL_HANDLE)
            return;

        vkUnmapMemory(device.device(), emitter.instanceData.memory);
        vkDestroyBuffer(device.device(), emitter.instanceData.buffer, nullptr);
        vkFreeMemory(device.device(), emitter.instanceData.memory, nullptr);

        emitter.instanceData.buffer = VK_NULL_HANDLE;
        emitter.instanceData.memory = VK_NULL_HANDLE;
        emitter.instanceData.mappedPointer = nullptr;
        emitter.instanceData.capacity = 0;
    }

    inline void destroyAllParticleEmitterInstanceData(EntityRegistry &registry)
    {
        registry.forEach<ParticleEmitterComponent>(
            [&](ParticleEmitterComponent &emitter)
            {
                destroyParticleEmitterInstanceData(emitter, registry.device);
            });
    }

    inline void updateParticleEmitters(EntityRegistry &registry, float frameTime)
    {
        registry.forEach<ParticleEmitterComponent, TransformComponent>(updateParticleEmitter, frameTime);
    }
}