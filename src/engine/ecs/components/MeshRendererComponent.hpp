#pragma once

#include "vModel.hpp"

#include <memory>

namespace ecs
{
    struct MeshRendererComponent
    {
        std::shared_ptr<v::vModel> model;
        bool active{true};
    };
}