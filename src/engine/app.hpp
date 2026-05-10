#pragma once

#include "defaultRenderSystem.hpp"
#include "hash.hpp"
#include "movementController.hpp"
#include "pointLightRenderSystem.hpp"
#include "vCamera.hpp"
#include "vDevice.hpp"
#include "vRenderer.hpp"
#include "vWindow.hpp"

#include "vDescriptorPool.hpp"
#include "vDescriptorSetLayout.hpp"
#include "vDescriptorWriter.hpp"

#include "EntityRegistry.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <chrono>
#include <memory>
#include <vector>

namespace v
{
    class vApp
    {
      public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        vApp();
        ~vApp();

        vApp(const vApp &) = delete;
        vApp &operator=(const vApp &) = delete;

        void run();

      private:
        void loadGameObjects();
        void updateCamera(float frameTime);

        void gui();

        vWindow window{WIDTH,
            HEIGHT,
            "GameEngine - " + std::string(__DATE__) + " " + std::string(__TIME__)};
        vDevice device{window};
        vRenderer renderer{window, device};

        std::unique_ptr<vDescriptorPool> globalDescriptorPool{};
        std::unique_ptr<vDescriptorPool> textureDescriptorPool{};
        std::unique_ptr<vDescriptorSetLayout> textureSetLayout{};

        float aspectRatio{1.0f};
        ecs::EntityRegistry entityRegistry{};
    };
}