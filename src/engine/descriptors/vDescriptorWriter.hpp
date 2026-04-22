#pragma once

#include "vDevice.hpp"
#include "vDescriptorWriter.hpp"
#include "vDescriptorPool.hpp"
#include "vDescriptorSetLayout.hpp"

#include <cassert>

namespace v
{


    class vDescriptorWriter
    {
      public:
        vDescriptorWriter(vDescriptorSetLayout &setLayout, vDescriptorPool &pool);

        vDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
        vDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

        bool build(VkDescriptorSet &set);
        void overwrite(VkDescriptorSet &set);

      private:
        vDescriptorSetLayout &setLayout;
        vDescriptorPool &pool;
        std::vector<VkWriteDescriptorSet> writes;
    };
}