#pragma once

#include "vCamera.hpp"

namespace ecs
{
    struct CameraComponent
    {
        v::vCamera camera{};
        float fovDegrees{80.f};
        float nearPlane{0.1f};
        float farPlane{1000.f};
        bool active{true};
    };
}
