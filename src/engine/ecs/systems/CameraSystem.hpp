#pragma once

#include "Camera.hpp"
#include "EntityStore.hpp"
#include "Transform.hpp"

namespace ecs
{
    class CameraSystem
    {
      public:
        static void update(EntityStore &store, float aspectRatio);
    };
}