#pragma once

#include "vDevice.hpp"

#include <cassert>
#include <memory>

namespace v {
        class vDescriptorSetLayout
    {
      public:
        class Builder
        {
          public:
            Builder(vDevice &device) : device{device} {}

            Builder &addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<vDescriptorSetLayout> build() const;

          private:
            vDevice &device;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        vDescriptorSetLayout(
            vDevice &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~vDescriptorSetLayout();
        vDescriptorSetLayout(const vDescriptorSetLayout &) = delete;
        vDescriptorSetLayout &operator=(const vDescriptorSetLayout &) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

      private:
        vDevice &device;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class vDescriptorWriter;
    };
}