#include "vGameObject.hpp"

namespace v {
    vGameObject vGameObject::makePointLight(float intensity, float radius, glm::vec3 color) {
        vGameObject gameObject = vGameObject::createGameObject();
        gameObject.pointLight = std::make_unique<PointLightComponent>();
        gameObject.pointLight->intensity = intensity;
        
        gameObject.color = color;
        gameObject.transform.scale.x = radius;

        return gameObject;
    }
}