#pragma once

#include "ComponentSet.hpp"

#include <functional>
#include <memory>
#include <queue>
#include <stdexcept>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
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
                entitiesInScene.insert(id);
                return id;
            }

            if (nextEntityId >= MAX_ENTITIES)
            {
                throw std::runtime_error("Maximum entity count exceeded");
            }

            nextEntityId++;
            entitiesInScene.insert(nextEntityId);
            return nextEntityId;
        }

        void destroyEntity(Entity entity)
        {
            entitiesInScene.erase(entity);
            for (auto &[type, set] : componentSets)
            {
                set->entityDestroyed(entity);
            }

            availableEntities.push(entity);
            entityNames.erase(entity);
        }

        const std::unordered_set<Entity> &getEntities() const
        {
            return entitiesInScene;
        }

        void setName(Entity entity, const std::string &name)
        {
            entityNames[entity] = name;
        }

        const std::string &getName(Entity entity) const
        {
            static const std::string unnamed = "Unnamed Entity" + std::to_string(entity);
            auto it = entityNames.find(entity);
            if (it == entityNames.end())
            {
                return unnamed;
            }
            return it->second;
        }

        template <typename Component>
        Component &addComponent(Entity entity, Component component)
        {
            auto &set = getComponentSet<Component>();
            set.insert(entity, std::move(component));
            return set.getComponentOf(entity);
        }

        template <typename Component>
        void removeComponent(Entity entity)
        {
            getComponentSet<Component>().remove(entity);
        }

        template <typename Component>
        Component &getComponent(Entity entity)
        {
            return getComponentSet<Component>().getComponentOf(entity);
        }

        template <typename Component>
        const Component &getComponent(Entity entity) const
        {
            return getComponentSet<Component>().getComponentOf(entity);
        }

        template <typename Component>
        Component *tryGetComponent(Entity entity)
        {
            auto key = std::type_index(typeid(Component));
            auto it = componentSets.find(key);
            if (it == componentSets.end())
                return nullptr;
            auto &set = *static_cast<ComponentSet<Component> *>(it->second.get());
            if (!set.has(entity))
                return nullptr;
            return &set.getComponentOf(entity);
        }

        template <typename Component>
        const Component *tryGetComponent(Entity entity) const
        {
            auto key = std::type_index(typeid(Component));
            auto it = componentSets.find(key);
            if (it == componentSets.end())
                return nullptr;
            auto &set = *static_cast<const ComponentSet<Component> *>(it->second.get());
            if (!set.has(entity))
                return nullptr;
            return &set.getComponentOf(entity);
        }

        template <typename Component>
        bool hasComponent(Entity entity) const
        {
            auto key = std::type_index(typeid(Component));
            auto set = componentSets.find(key);
            if (set == componentSets.end())
                return false;
            return static_cast<const ComponentSet<Component> *>(set->second.get())->has(entity);
        }

        // first component should be specific to what you want, other components are just to give access (usually transform)
        template <typename FirstComponent, typename... Components, typename FunctionToCall, typename... Args>
        void forEach(FunctionToCall &&function, Args &&...args)
        {
            auto &firstSet = getComponentSet<FirstComponent>();
            auto secondarySets = std::make_tuple(&getComponentSet<Components>()...);

            for (size_t i = 0; i < firstSet.size(); i++)
            {
                Entity entity = firstSet.entities()[i];
                if ((std::get<ComponentSet<Components> *>(secondarySets)->has(entity) && ...))
                {
                    FirstComponent &first = firstSet.getByIndex(i);
                    if constexpr (std::is_invocable_v<FunctionToCall, Entity, FirstComponent &, Components &..., Args &&...>) // function (entity, components, args...)
                    {
                        std::invoke(std::forward<FunctionToCall>(function), entity, first,
                            std::get<ComponentSet<Components> *>(secondarySets)->getComponentOf(entity)...,
                            std::forward<Args>(args)...);
                    }
                    else // function (components, args...)
                    {
                        std::invoke(std::forward<FunctionToCall>(function), first,
                            std::get<ComponentSet<Components> *>(secondarySets)->getComponentOf(entity)...,
                            std::forward<Args>(args)...);
                    }
                }
            }
        }

      private:
        uint32_t nextEntityId = 0;
        std::queue<Entity> availableEntities;
        std::unordered_map<std::type_index, std::unique_ptr<BaseComponentSet>> componentSets;

        std::unordered_map<Entity, std::string> entityNames;
        std::unordered_set<Entity> entitiesInScene;

        template <typename Component>
        const ComponentSet<Component> &getComponentSet() const // prolly dont need this cuz i always forget to use const. oops. but uhh just keeping here in case
        {
            auto key = std::type_index(typeid(Component));
            auto set = componentSets.find(key);
            if (set == componentSets.end())
            {
                throw std::runtime_error("Component set not found (const)");
            }
            return *static_cast<const ComponentSet<Component> *>(set->second.get());
        }

        template <typename Component>
        ComponentSet<Component> &getComponentSet()
        {
            auto key = std::type_index(typeid(Component));
            auto set = componentSets.find(key);
            if (set == componentSets.end())
            {
                componentSets[key] = std::make_unique<ComponentSet<Component>>();
                return *static_cast<ComponentSet<Component> *>(componentSets[key].get());
            }
            return *static_cast<ComponentSet<Component> *>(set->second.get());
        }
    };
}