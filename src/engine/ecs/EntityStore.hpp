#pragma once
#include "EntityHandle.hpp"

#include "MeshRenderer.hpp"
#include "PointLight.hpp"
#include "Transform.hpp"

#include <utility>

namespace ecs {
    class EntityStore
    {
      public:
        EntityHandle createEntity() { return EntityHandle{registry, registry.createEntity()}; }
        void destroyEntity(EntityHandle entity) { registry.destroyEntity(entity.id()); }

        template <typename FirstComponent, typename... Components, typename FunctionToCall, typename... Args>
        void forEach(FunctionToCall&& function, Args&&... args)
        {
            registry.template forEach<FirstComponent, Components...>(std::forward<FunctionToCall>(function), std::forward<Args>(args)...);
        }

      private:
        EntityRegistry registry;
    };
}