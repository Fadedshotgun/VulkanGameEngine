#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace ecs
{
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
        std::vector<Particle> particles;
        float timeSinceLastEmission{0};
    };
}