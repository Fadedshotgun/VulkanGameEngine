#include "CameraSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace ecs
{
    static void updateCamera(CameraComponent &cameraComponent, TransformComponent &transform, float aspectRatio)
    {
        if (!cameraComponent.active)
        {
            return;
        }

        cameraComponent.camera.setViewYXZ(transform.translation, transform.rotation);
        cameraComponent.camera.setPerspectiveProjection(glm::radians(cameraComponent.fovDegrees), aspectRatio, cameraComponent.nearPlane, cameraComponent.farPlane);
    }

    void CameraSystem::update(EntityStore &store, float aspectRatio)
    {

        store.forEach<CameraComponent, TransformComponent>(updateCamera, aspectRatio);
    }
}
