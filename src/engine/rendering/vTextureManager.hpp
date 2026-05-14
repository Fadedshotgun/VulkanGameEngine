#pragma once

#include "vDescriptorPool.hpp"
#include "vDescriptorSetLayout.hpp"
#include "vDescriptorWriter.hpp"
#include "vDevice.hpp"
#include "vTexture.hpp"

namespace v
{
    class vTextureManager
    {
      public:
        vTextureManager(vDevice &device, vDescriptorSetLayout &textureSetLayout, vDescriptorPool &textureDescriptorPool);
        ~vTextureManager();

        std::pair<VkDescriptorSet, std::shared_ptr<vTexture>> loadTexture(std::string texturePath);
        bool isTextureLoaded(std::string texturePath) { return loadedTextures.find(texturePath) != loadedTextures.end(); }
        std::pair<VkDescriptorSet, std::shared_ptr<vTexture>> getTexture(std::string texturePath) { return loadedTextures[texturePath]; }

        std::unordered_map<std::string, std::pair<VkDescriptorSet, std::shared_ptr<vTexture>>> loadedTextures;

      private:
        vDevice &device;
        vDescriptorSetLayout &textureSetLayout;
        vDescriptorPool &textureDescriptorPool;
    };
}