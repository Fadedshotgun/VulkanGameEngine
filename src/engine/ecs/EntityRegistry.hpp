#pragma once

#include "ComponentSet.hpp"

#include <memory>
#include <queue>
#include <typeindex>
#include <unordered_map>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace ecs
{
    class EntityRegistry
    {
      public:
                static constexpr Entity MAX_ENTITIES = 5000;

        Entity createEntity()
        {
            if (!availableEntities.empty())
            {
                Entity id = availableEntities.front();
                availableEntities.pop();
                return id;
            }

            if (nextEntityId >= MAX_ENTITIES)
            {
                throw std::runtime_error("Maximum entity count exceeded");
            }

            return nextEntityId++;
        }

        void destroyEntity(Entity entity)
        {
            for (auto &[type, set] : componentSets)
            {
                set->entityDestroyed(entity);
            }

            availableEntities.push(entity);
        }

        template <typename Component>
        Component& addComponent(Entity entity, Component component)
        {
            auto& set = getComponentSet<Component>();
            set.insert(entity, std::move(component));
            return set.getComponentOf(entity);
        }

        template <typename Component>
        void removeComponent(Entity entity)
        {
            getComponentSet<Component>().remove(entity);
        }

        template <typename Component>
        Component& getComponent(Entity entity)
        {
            return getComponentSet<Component>().getComponentOf(entity);
        }

        template <typename Component>
        const Component& getComponent(Entity entity) const
        {
            return getComponentSet<Component>().getComponentOf(entity);
        }

        template <typename Component>
        Component& tryGetComponent(Entity entity)
        {
            auto& set = getComponentSet<Component>();
            if (!set.has(entity))
            {
                return nullptr;
            }
            return set.getComponentOf(entity);
        }

        template <typename Component>
        const Component& tryGetComponent(Entity entity) const
        {
            auto& set = getComponentSet<Component>();
            if (!set.has(entity))
            {
                return nullptr;
            }
            return set.getComponentOf(entity);
        }

        template <typename Component>
        bool hasComponent(Entity entity) const
        {
            auto key = std::type_index(typeid(Component));
            auto set  = componentSets.find(key);
            if (set == componentSets.end()) return false;
            return static_cast<const ComponentSet<Component>*>(set->second.get())->has(entity);
        }

        template <typename FirstComponent, typename... Components, typename FunctionToCall, typename... Args>
        void forEach(FunctionToCall&& function, Args&&... args)
        {
            auto& firstSet = getComponentSet<FirstComponent>();
            for (Entity entity : firstSet.entities())
            {
                if ((hasComponent<Components>(entity) && ...))
                {
                    auto &first = firstSet.getComponentOf(entity);

                    if constexpr (std::is_invocable_v<FunctionToCall, Entity, FirstComponent&, Components&..., Args&&...>) // function (entity, components, args...)
                    {
                        std::invoke(std::forward<FunctionToCall>(function), entity, first, getComponent<Components>(entity)..., std::forward<Args>(args)...);
                    }
                    else // function (components, args...)
                    {
                        std::invoke(std::forward<FunctionToCall>(function), first, getComponent<Components>(entity)..., std::forward<Args>(args)...);
                    }
                }
            }
        }

      private:
        uint32_t nextEntityId = 0;
        std::queue<Entity> availableEntities;
        std::unordered_map<std::type_index, std::unique_ptr<BaseComponentSet>> componentSets;

        template <typename Component>
        const ComponentSet<Component>& getComponentSet() const
        {
            auto key = std::type_index(typeid(Component));
            return *static_cast<const ComponentSet<Component>*>(componentSets.at(key).get());
        }

        template <typename Component>
        ComponentSet<Component>& getComponentSet()
        {
            auto key = std::type_index(typeid(Component));
            auto set = componentSets.find(key);
            if (set == componentSets.end())
            {
                componentSets[key] = std::make_unique<ComponentSet<Component>>();
                return *static_cast<ComponentSet<Component>*>(componentSets[key].get());
            }
            return *static_cast<ComponentSet<Component>*>(set->second.get());
        }
    };
}