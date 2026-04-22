#pragma once

#include "vModel.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>

namespace v
{
    struct TransformComponent
    {
        glm::vec3 translation{};
        glm::vec3 scale{1.f, 1.f, 1.f};
        glm::vec3 rotation;

        glm::mat4 mat4()
        {
            const float c3 = glm::cos(rotation.z);
            const float s3 = glm::sin(rotation.z);
            const float c2 = glm::cos(rotation.x);
            const float s2 = glm::sin(rotation.x);
            const float c1 = glm::cos(rotation.y);
            const float s1 = glm::sin(rotation.y);
            return glm::mat4{
                {
                    scale.x * (c1 * c3 + s1 * s2 * s3),
                    scale.x * (c2 * s3),
                    scale.x * (c1 * s2 * s3 - c3 * s1),
                    0.0f,
                },
                {
                    scale.y * (c3 * s1 * s2 - c1 * s3),
                    scale.y * (c2 * c3),
                    scale.y * (c1 * c3 * s2 + s1 * s3),
                    0.0f,
                },
                {
                    scale.z * (c2 * s1),
                    scale.z * (-s2),
                    scale.z * (c1 * c2),
                    0.0f,
                },
                {translation.x, translation.y, translation.z, 1.0f}};
        }
    };

    struct PointLightComponent {
        float intensity{1.f};
    };

    class vGameObject
    {
      public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, vGameObject>;

        static vGameObject createGameObject()
        {
            static id_t currentId = 0;
            return vGameObject(currentId++);
        }

        static vGameObject makePointLight(float intensity = 1.f, float radius = .1f, glm::vec3 color = {1.f, 1.f, 1.f});

        vGameObject(const vGameObject &) = delete;
        vGameObject &operator=(const vGameObject &) = delete;
        vGameObject(vGameObject &&) = default;
        vGameObject &operator=(vGameObject &&) = default;

        id_t getId() { return id; }

        glm::vec3 color{};
        TransformComponent transform{};

        // optional components
        std::shared_ptr<vModel> model;
        std::unique_ptr<PointLightComponent> pointLight = nullptr;

      private:
        vGameObject(id_t objectId) : id(objectId) {}

        id_t id;
    };
}