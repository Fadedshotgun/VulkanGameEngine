#include <memory>

#include <vulkan/vulkan.h>

#include "vDescriptorPool.hpp"
#include "vDevice.hpp"
#include "vRenderer.hpp"
#include "vTextureManager.hpp"
#include "vWindow.hpp"

#include "entityRegistry.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

namespace editor
{
    class EditorUI
    {
      public:
        EditorUI(v::vDevice &device, v::vWindow &window, v::vRenderer &renderer, v::vTextureManager &textureManager);
        ~EditorUI();

        bool drawUI(ecs::EntityRegistry &entityRegistry, int multiplier);
        void render(VkCommandBuffer commandBuffer);
        void updateView(VkCommandBuffer commandBuffer, float &aspectRatio);
        void selectEntity(Entity id) { currentlySelectedEntity = id; }

        std::pair<VkDescriptorSet, std::shared_ptr<v::vTexture>> loadTexture(std::string texturePath, v::vDescriptorSetLayout &textureSetLayout, v::vDescriptorPool &textureDescriptorPool);

      private:
        void inspector(ecs::EntityRegistry &entityRegistry);
        void explorer();
        void buttonTest();

        std::unique_ptr<v::vDescriptorPool> imguiDescriptorPool{};
        v::vRenderer &renderer;
        v::vTextureManager &textureManager;
        ImGuiID dockspaceId{0};
        Entity currentlySelectedEntity{0};
    };
}