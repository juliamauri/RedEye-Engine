/*
 * RedEye Engine - A 3D Game Engine written in C++.
 * Copyright (C) 2018-2024 Julia Mauri and Ruben Sardon
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

module;

#include <vulkan/vulkan.h>

export module Uniforms;

import Buffers;

export struct UniformCreateInfo
{
    void* data = nullptr;
    uint32_t binding = 0;
    VkDeviceSize size = 0;

    template <typename T> static UniformCreateInfo From(T& data, uint32_t binding = 0)
    {
        return {&data, binding, sizeof(T)};
    }
};

export struct Uniform
{
    void* data = nullptr;
    Buffer buffer{};

    bool Create(const VkDevice logical_device, VkPhysicalDeviceMemoryProperties mem_properties,
                const VkDescriptorPool& descriptorPool,
                const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts,
                VkDescriptorSet& descriptor_set, UniformCreateInfo create_info, VkDeviceSize buffer_offset = 0)
    {
        // Create Uniform Buffer
        if (!buffer.Create(logical_device, mem_properties, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, create_info.size,
                           buffer_offset) ||
            !buffer.FillData(logical_device, data = create_info.data))
            return false;

        // Create Descriptor Set
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(descriptor_set_layouts.size());
        allocInfo.pSetLayouts = descriptor_set_layouts.data();

        if (vkAllocateDescriptorSets(logical_device, &allocInfo, &descriptor_set) != VK_SUCCESS)
            return false;

        // Update Descriptor Set
        VkDescriptorBufferInfo bufferInfo = {buffer.id, 0, buffer.stride};
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptor_set;
        descriptorWrite.dstBinding = create_info.binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        vkUpdateDescriptorSets(logical_device, 1, &descriptorWrite, 0, nullptr);
        return true;
    }

    bool Update(VkDevice logical_device) const
    {
        return data != nullptr && buffer.FillData(logical_device, data);
    }

    void Clear(VkDevice logical_device)
    {
        buffer.Clear(logical_device);
    }
};