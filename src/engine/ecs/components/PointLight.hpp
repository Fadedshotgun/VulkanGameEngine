#pragma once

#include <glm/glm.hpp>

namespace ecs
{
    struct PointLightComponent
    {
        glm::vec3 color{};
        float intensity{1.f};
    };
}