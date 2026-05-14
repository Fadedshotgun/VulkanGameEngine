#pragma once

#include "Components.hpp"
#include "EntityRegistry.hpp"

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

    static void emitParticle(ParticleEmitterComponent &emitter, TransformComponent &transform, int count = 1)
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

    static void clampEmitter(ParticleEmitterComponent &emitter)
    {
        emitter.size = std::max(0.f, emitter.size);
        emitter.emissionRate = std::max(0.f, emitter.emissionRate);
        emitter.lifetime = std::max(0.f, emitter.lifetime);
        emitter.angle1 = std::max(0.f, emitter.angle1);
    }

    static void updateParticleEmitter(ParticleEmitterComponent &emitter, TransformComponent &transform, float frameTime)
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

    void updateParticleEmitters(EntityRegistry &registry, float frameTime)
    {
        registry.forEach<ParticleEmitterComponent, TransformComponent>(updateParticleEmitter, frameTime);
    }
}