#pragma once

#include "vDevice.hpp"

#include <memory>

namespace v
{
    class vDescriptorPool
    {
      public:
        class Builder
        {
          public:
            Builder(vDevice &device) : device{device} {}

            Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder &setMaxSets(uint32_t count);
            std::unique_ptr<vDescriptorPool> build() const;

          private:
            vDevice &device;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        vDescriptorPool(
            vDevice &device,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize> &poolSizes);
        ~vDescriptorPool();
        vDescriptorPool(const vDescriptorPool &) = delete;
        vDescriptorPool &operator=(const vDescriptorPool &) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

        void resetPool();
        VkDescriptorPool getDescriptorPool() const { return descriptorPool; }

      private:
        vDevice &device;
        VkDescriptorPool descriptorPool;

        friend class vDescriptorWriter;
    };
}
