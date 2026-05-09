#pragma once

#include "CameraComponent.hpp"
#include "EntityRegistry.hpp"
#include "TransformComponent.hpp"

namespace ecs
{
    class CameraSystem
    {
      public:
        static void update(EntityRegistry &registry, float aspectRatio);
    };
}