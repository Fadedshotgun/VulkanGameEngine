#pragma once

#include "vDevice.hpp"
#include "vModel.hpp"

#include <string>
#include <vector>

namespace v
{

    struct PipelineConfigInfo
    {
        PipelineConfigInfo() = default;
        PipelineConfigInfo(const PipelineConfigInfo &) = delete;
        PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

        std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;

        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;

        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class vPipeline
    {
      public:
        vPipeline(vDevice &device, const PipelineConfigInfo &config, const std::string &vertexFilePath, const std::string &fragmentFilePath);
        ~vPipeline();

        vPipeline(const vPipeline &) = delete;
        vPipeline &operator=(const vPipeline &) = delete;

        void bind(VkCommandBuffer commandBuffer);
        static void defaultPipelineConfigInfo(PipelineConfigInfo &configInfo);
        static void enableAlphaBlending(PipelineConfigInfo &configInfo);

      private:
        static std::vector<char> readFile(const std::string &filePath);

        void createGraphicsPipeline(const PipelineConfigInfo &config, const std::string &vertexFilePath, const std::string &fragmentFilePath);

        void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);

        vDevice &device;
        VkPipeline graphicsPipeline;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
    };
}