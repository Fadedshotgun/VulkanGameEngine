#include "vTextureManager.hpp"

namespace v
{
    vTextureManager::vTextureManager(vDevice &device, vDescriptorSetLayout &textureSetLayout, vDescriptorPool &textureDescriptorPool) : device(device), textureSetLayout(textureSetLayout), textureDescriptorPool(textureDescriptorPool) {}

    vTextureManager::~vTextureManager() {}

    std::pair<VkDescriptorSet, std::shared_ptr<vTexture>> vTextureManager::loadTexture(std::string texturePath)
    {
        if (isTextureLoaded(texturePath))
        {
            return getTexture(texturePath);
        }

        auto newTexture = std::make_shared<vTexture>(device, texturePath);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = newTexture->getImageView();
        imageInfo.sampler = newTexture->getSampler();

        VkDescriptorSet textureSet = VK_NULL_HANDLE;

        vDescriptorWriter descriptorWriter(textureSetLayout, textureDescriptorPool);
        descriptorWriter.writeImage(0, &imageInfo);
        descriptorWriter.build(textureSet);

        loadedTextures[texturePath] = std::make_pair(textureSet, newTexture);
        return loadedTextures[texturePath];
    }
}