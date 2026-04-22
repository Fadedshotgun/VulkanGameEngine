#include "vDescriptorPool.hpp"

namespace v
{
    vDescriptorPool::Builder &vDescriptorPool::Builder::addPoolSize(
        VkDescriptorType descriptorType, uint32_t count)
    {
        poolSizes.push_back({descriptorType, count});
        return *this;
    }

    vDescriptorPool::Builder &vDescriptorPool::Builder::setPoolFlags(
        VkDescriptorPoolCreateFlags flags)
    {
        poolFlags = flags;
        return *this;
    }
    vDescriptorPool::Builder &vDescriptorPool::Builder::setMaxSets(uint32_t count)
    {
        maxSets = count;
        return *this;
    }

    std::unique_ptr<vDescriptorPool> vDescriptorPool::Builder::build() const
    {
        return std::make_unique<vDescriptorPool>(device, maxSets, poolFlags, poolSizes);
    }

    vDescriptorPool::vDescriptorPool(vDevice &device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize> &poolSizes) : device{device}
    {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;

        if (vkCreateDescriptorPool(device.device(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    vDescriptorPool::~vDescriptorPool()
    {
        vkDestroyDescriptorPool(device.device(), descriptorPool, nullptr);
    }

    bool vDescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;


        if (vkAllocateDescriptorSets(device.device(), &allocInfo, &descriptor) != VK_SUCCESS)
        {
            // this is quite bad
            std::cerr << "Failed to allocate descriptor set! pool is probably full" << std::endl;
            return false;
        }
        return true;
    }

    void vDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const
    {
        vkFreeDescriptorSets(
            device.device(),
            descriptorPool,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
    }

    void vDescriptorPool::resetPool()
    {
        vkResetDescriptorPool(device.device(), descriptorPool, 0);
    }
}
