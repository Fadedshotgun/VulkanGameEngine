#pragma once

#include "Entity.hpp"

#include <vector>
#include <utility>
#include <cstddef>
#include <cassert>
#include <limits>

namespace ecs
{
    class BaseComponentSet
    {
      public:
        virtual ~BaseComponentSet() = default;
        virtual void entityDestroyed(Entity entity) = 0;
    };

    template <typename Component>
    class ComponentSet : public BaseComponentSet
    {
      public:
        static constexpr Entity INVALID_INDEX = std::numeric_limits<Entity>::max();

        ComponentSet() = default;

        Component &getComponentOf(Entity entity)
        {
            assert(has(entity) && "Component not present on entity");
            return denseComponents[sparseSet[entity]];
        };
        const Component &getComponentOf(Entity entity) const
        {
            assert(has(entity) && "Component not present on entity");
            return denseComponents[sparseSet[entity]];
        };

        bool has(Entity entity) const { return entity < sparseSet.size() && sparseSet[entity] != INVALID_INDEX; };

        const std::vector<Entity> &entities() const { return denseSet; }
        size_t size() const { return denseSet.size(); }

        Component &getByIndex(size_t i)
        {
            assert(i < denseComponents.size() && "Index out of range");
            return denseComponents[i];
        }
        const Component &getByIndex(size_t i) const
        {
            assert(i < denseComponents.size() && "Index out of range");
            return denseComponents[i];
        }

        void insert(Entity entity, Component component)
        {
            if (has(entity))
            {
                return;
            }

            if (entity >= sparseSet.size())
            {
                sparseSet.resize(entity + 1, INVALID_INDEX);
            }

            sparseSet[entity] = denseSet.size();
            denseSet.push_back(entity);
            denseComponents.push_back(std::move(component));
        }

        void remove(Entity entity)
        {
            if (!has(entity))
            {
                return;
            }

            size_t index = sparseSet[entity];
            size_t lastIndex = denseSet.size() - 1;

            if (index != lastIndex)
            {
                Entity lastEntity = denseSet[lastIndex];
                denseSet[index] = lastEntity;
                denseComponents[index] = std::move(denseComponents[lastIndex]);
                sparseSet[lastEntity] = index;
            }

            sparseSet[entity] = INVALID_INDEX;
            denseSet.pop_back();
            denseComponents.pop_back();
        }

        void entityDestroyed(Entity entity) override
        {
            if (has(entity))
            {
                remove(entity);
            }
        }
      private:
        std::vector<Entity> sparseSet;

        std::vector<Entity> denseSet;
        std::vector<Component> denseComponents;
    };
};
