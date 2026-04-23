#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "vDevice.hpp"
#include "vBuffer.hpp"
#include "vTexture.hpp"
#include "vDescriptorSetLayout.hpp"
#include "vDescriptorPool.hpp"

#include <memory>
#include <vector>

namespace v
{
    class vModel
    {
      public:
        struct Vertex
        {
            glm::vec3 position{};
            glm::vec3 color{};
            glm::vec3 normal{};
            glm::vec2 uv{};

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

            bool operator==(const Vertex &other) const
            {
                return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
            }
        };

        struct Data
        {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};
            std::string diffuseTexturePath{};

            void loadModel(const std::string &filepath);
        };

        struct UniformBufferObject
        {
            glm::mat4 proj;
        };

        vModel(vDevice &device, const vModel::Data &modelData);
        ~vModel();

        vModel(const vModel &) = delete;
        vModel &operator=(const vModel &) = delete;

        static std::unique_ptr<vModel> createModelFromFile(vDevice &device, const std::string &filepath);
        static std::unique_ptr<vModel> createModelFromFile(vDevice &device, const std::string &filepath, const std::string &texturePath);
        static std::shared_ptr<vModel> createSharedModelFromFile(vDevice &device, const std::string &filepath, vDescriptorSetLayout &setLayout, vDescriptorPool &pool);
        static std::shared_ptr<vModel> createSharedModelFromFile(vDevice &device, const std::string &filepath, const std::string &texturePath, vDescriptorSetLayout &setLayout, vDescriptorPool &pool); 

        void setTexture(std::shared_ptr<vTexture> texture);
        std::shared_ptr<vTexture> getTexture() const { return texture; }
        bool hasTexture() const { return texture != nullptr; }
        bool createTextureDescriptor(vDescriptorSetLayout &setLayout, vDescriptorPool &pool);
        VkDescriptorSet getTextureDescriptorSet() const { return textureDescriptorSet; }
        bool hasTextureDescriptorSet() const { return textureDescriptorSet != VK_NULL_HANDLE; }

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

        static std::unique_ptr<vModel> createCubeModel(vDevice &device, glm::vec3 offset);

      private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<uint32_t> &indices); // CAN CHANGE TO UINT16_T FOR LESS VERTICES <- liar I cant anymore cuz im building chunks which have many vertices :c

        vDevice &device;

        std::unique_ptr<vBuffer> vertexBuffer;
        std::unique_ptr<vBuffer> indexBuffer;
        uint32_t indexCount;
        uint32_t vertexCount;

        bool hasIndexBuffer = false;
        std::shared_ptr<vTexture> texture{};
        VkDescriptorSet textureDescriptorSet = VK_NULL_HANDLE;
        
    };
}