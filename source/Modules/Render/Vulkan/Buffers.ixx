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

#include <iostream>
#include <vector>

export module Buffers;

import VkDebug;

export struct Buffer
{
    VkBuffer id = VK_NULL_HANDLE;
    VkDeviceSize buffer_size = 0;
    uint32_t element_count = 0;
    VkDeviceMemory memory = VK_NULL_HANDLE;

    template <typename T>
    bool CreateFromStagedCopy(VkDevice logical_device, VkQueue queue,
                              const VkPhysicalDeviceMemoryProperties& mem_properties, VkCommandPool commandPool,
                              VkBufferUsageFlagBits usage_flags, const std::vector<T>& vector)
    {
        element_count = static_cast<uint32_t>(vector.size());
        buffer_size = static_cast<VkDeviceSize>(sizeof(vector[0])) * element_count;

        Buffer staging_buffer;
        if (!staging_buffer.Create(logical_device, mem_properties, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
        {
            std::cerr << "Failed to create Staging Buffer." << std::endl;
            return false;
        }

        if (!staging_buffer.FillData(logical_device, vector.data()))
        {
            std::cerr << "Failed to copy data to Staging Buffer." << std::endl;
            return false;
        }
        if (!Create(logical_device, mem_properties, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage_flags,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
        {
            std::cerr << "Failed to create new Buffer." << std::endl;
            return false;
        }
        if (!staging_buffer.CopyTo(logical_device, id, buffer_size, queue, commandPool))
            return false;

        staging_buffer.Clear(logical_device);
        return true;
    }

    bool Create(VkDevice device, const VkPhysicalDeviceMemoryProperties& mem_properties, VkDeviceSize size,
                VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    {
        VkBufferCreateInfo buffer_info{};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = buffer_size = size;
        buffer_info.usage = usage;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &buffer_info, nullptr, &id) != VK_SUCCESS)
        {
            std::cerr << "Failed to create buffer!" << std::endl;
            return false;
        }

        VkMemoryRequirements mem_requirements;
        vkGetBufferMemoryRequirements(device, id, &mem_requirements);

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_requirements.size;

        bool found_valid_mem_type = false;
        for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
        {
            if ((mem_requirements.memoryTypeBits & (1 << i)) &&
                (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                found_valid_mem_type = true;
                alloc_info.memoryTypeIndex = i;
                break;
            }
        }
        if (!found_valid_mem_type)
        {
            std::cerr << "Failed to find suitable memory type!" << std::endl;
            return false;
        }

        if (vkAllocateMemory(device, &alloc_info, nullptr, &memory) != VK_SUCCESS)
        {
            std::cerr << "Failed to allocate buffer memory!" << std::endl;
            return false;
        }

        if (vkBindBufferMemory(device, id, memory, 0) != VK_SUCCESS)
        {
            std::cerr << "Failed to bind buffer memory!" << std::endl;
            return false;
        }

        return true;
    }

    void Clear(VkDevice logical_device)
    {
        if (id != VK_NULL_HANDLE)
            vkDestroyBuffer(logical_device, id, VkDebug::Allocation());

        if (memory != VK_NULL_HANDLE)
            vkFreeMemory(logical_device, memory, VkDebug::Allocation());
    }

    bool FillData(VkDevice device, const void* source) const
    {
        void* tmp;
        if (vkMapMemory(device, memory, 0, buffer_size, 0, &tmp) != VK_SUCCESS)
        {
            std::cerr << "Failed to map buffer data for copy operation." << std::endl;
            return false;
        }
        memcpy(tmp, source, static_cast<size_t>(buffer_size));
        vkUnmapMemory(device, memory);
        return true;
    }

    bool CopyTo(VkDevice device, VkBuffer dst_buffer, VkDeviceSize size, VkQueue queue, VkCommandPool commandPool) const
    {
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = commandPool;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer command_buffer;
        if (vkAllocateCommandBuffers(device, &alloc_info, &command_buffer) != VK_SUCCESS)
        {
            std::cerr << "Failed to allocate command buffer in Buffer::CopyTo(...)." << std::endl;
            return false;
        }

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
        {
            std::cerr << "Failed to begin command buffer in Buffer::CopyTo(...)." << std::endl;
            return false;
        }

        VkBufferCopy copy_region{};
        copy_region.size = size;
        vkCmdCopyBuffer(command_buffer, id, dst_buffer, 1, &copy_region);

        if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
        {
            std::cerr << "Failed to end command buffer in Buffer::CopyTo(...)." << std::endl;
            return false;
        }

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;

        if (vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS)
        {
            std::cerr << "Failed to submit command buffer to queue in Buffer::CopyTo(...)." << std::endl;
            return false;
        }

        if (vkQueueWaitIdle(queue) != VK_SUCCESS)
        {
            std::cerr << "Failed to wait for idle queue in Buffer::CopyTo(...)." << std::endl;
            return false;
        }

        vkFreeCommandBuffers(device, commandPool, 1, &command_buffer);
        return true;
    }
};
