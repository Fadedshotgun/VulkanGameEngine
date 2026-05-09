#pragma once

#include "EntityRegistry.hpp"

// TODO MAKE SAFER
namespace ecs
{
    class EntityHandle
    {
      public:
        EntityHandle(EntityRegistry &registry, Entity entity) : registry{registry}, entity{entity} {}

        template <typename Component>
        Component &getComponent() { return registry.getComponent<Component>(entity); }

        template <typename Component>
        const Component &getComponent() const { return registry.getComponent<Component>(entity); }

        template <typename Component>
        bool hasComponent() const { return registry.hasComponent<Component>(entity); }

        template <typename Component>
        Component &addComponent(Component component) { return registry.addComponent<Component>(entity, std::move(component)); }

        template <typename Component>
        void removeComponent() { registry.removeComponent<Component>(entity); }

        Entity id() const { return entity; }

      private:
        EntityRegistry &registry;
        Entity entity;
    };
}
