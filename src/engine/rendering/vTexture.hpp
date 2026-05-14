#pragma once

#include <string>

#include "vBuffer.hpp"
#include "vDevice.hpp"

namespace v
{
    class vTexture
    {
      public:
        vTexture(vDevice &device, const std::string &texturePath);
        ~vTexture();

        vTexture(const vTexture &) = delete;
        vTexture &operator=(const vTexture &) = delete;

        VkImageView getImageView() const { return textureImageView; }
        VkSampler getSampler() const { return textureSampler; }

      private:
        void transitionImageLayout(vDevice &device, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

        void createTextureImage(const std::string &texturePath);
        void createImage(vBuffer &stagingBuffer, int texWidth, int texHeight);

        void createTextureImageView();
        void createTextureSampler();

        vDevice &device;
        VkImage textureImage = VK_NULL_HANDLE;
        VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
        VkImageView textureImageView = VK_NULL_HANDLE;
        VkSampler textureSampler = VK_NULL_HANDLE;
    };
}