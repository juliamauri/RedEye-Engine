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
    VkDeviceSize stride = 0; // Size of the buffer in bytes, used for data transfer and memory allocation.
    VkDeviceSize offset = 0; // Offset in bytes from the start of the buffer where data will be written or read.
    VkDeviceMemory memory = VK_NULL_HANDLE;

    template <typename T>
    bool CreateFromStagedCopy(VkDevice logical_device, VkQueue queue,
                              const VkPhysicalDeviceMemoryProperties& mem_properties, VkCommandPool commandPool,
                              VkBufferUsageFlagBits usage_flags, const std::vector<T>& vector, VkDeviceSize buffer_offset)
    {
        Buffer staging_buffer{};
        if (!staging_buffer.Create(logical_device, mem_properties, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   sizeof(T) * vector.size(), buffer_offset))
        {
            std::cerr << "Failed to create Staging Buffer." << std::endl;
            return false;
        }

        if (!staging_buffer.FillData(logical_device, vector.data()))
        {
            std::cerr << "Failed to copy data to Staging Buffer." << std::endl;
            return false;
        }
        if (!Create(logical_device, mem_properties, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage_flags,
                    staging_buffer.stride, staging_buffer.offset, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
        {
            std::cerr << "Failed to create new Buffer." << std::endl;
            return false;
        }
        if (!staging_buffer.CopyTo(logical_device, queue, commandPool, *this))
            return false;

        staging_buffer.Clear(logical_device);
        return true;
    }

    bool Create(VkDevice logical_device, const VkPhysicalDeviceMemoryProperties& mem_properties,
        VkBufferUsageFlags usage, 
        VkDeviceSize buffer_size, 
        VkDeviceSize buffer_offset = 0,
        VkMemoryPropertyFlags mem_property_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    {
        stride = buffer_size;
        offset = buffer_offset;

        VkBufferCreateInfo buffer_info{};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = stride;
        buffer_info.usage = usage;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(logical_device, &buffer_info, VkDebug::Allocation(), &id) != VK_SUCCESS)
        {
            std::cerr << "Failed to create buffer!" << std::endl;
            return false;
        }

        VkMemoryRequirements mem_requirements;
        vkGetBufferMemoryRequirements(logical_device, id, &mem_requirements);

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_requirements.size;
        if (!FindMemoryType(mem_properties, mem_property_flags, mem_requirements, alloc_info.memoryTypeIndex))
        {
            std::cerr << "Failed to find suitable memory type!" << std::endl;
            Clear(logical_device);
            return false;
        }

        if (vkAllocateMemory(logical_device, &alloc_info, VkDebug::Allocation(), &memory) != VK_SUCCESS)
        {
            std::cerr << "Failed to allocate buffer memory!" << std::endl;
            Clear(logical_device);
            return false;
        }

        if (vkBindBufferMemory(logical_device, id, memory, offset) != VK_SUCCESS)
        {
            std::cerr << "Failed to bind buffer memory!" << std::endl;
            Clear(logical_device);
            return false;
        }

        return true;
    }

    void Clear(VkDevice logical_device)
    {
        if (id == VK_NULL_HANDLE)
            return;

        vkDestroyBuffer(logical_device, id, VkDebug::Allocation());
        vkFreeMemory(logical_device, memory, VkDebug::Allocation());
        id = VK_NULL_HANDLE;
        memory = VK_NULL_HANDLE;
    }

    bool FillData(VkDevice device, const void* source, VkMemoryMapFlags flags = 0) const
    {
        void* tmp;
        if (vkMapMemory(device, memory, offset, stride, flags, &tmp) != VK_SUCCESS)
        {
            std::cerr << "Failed to map buffer data for copy operation." << std::endl;
            return false;
        }
        memcpy(tmp, source, static_cast<size_t>(stride));
        vkUnmapMemory(device, memory);
        return true;
    }

    private:

    bool FindMemoryType(const VkPhysicalDeviceMemoryProperties& mem_properties,
                          VkMemoryPropertyFlags mem_property_flags, const VkMemoryRequirements& mem_requirements,
                          uint32_t& memory_type_id) const
    {
        for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
        {
            if ((mem_requirements.memoryTypeBits & (1 << i)) &&
                (mem_properties.memoryTypes[i].propertyFlags & mem_property_flags) == mem_property_flags)
            {
                memory_type_id = i;
                return true;
            }
        }
        return false;
    }

    bool CopyTo(VkDevice device, VkQueue queue, VkCommandPool commandPool, const Buffer& dst) const
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
        copy_region.srcOffset = offset;
        copy_region.dstOffset = dst.offset;
        if (stride != dst.stride)
        {
            copy_region.size = std::min(stride, dst.stride);
            std::cout << "Warning: Buffer::CopyTo(...) will only copy " << copy_region.size << "/"
                      << std::max(stride, dst.stride) << " bytes. Src Stride : (" << stride << ") Dst Stride ("
                      << dst.stride << ")" << std::endl;
        }
        else copy_region.size = stride;

        vkCmdCopyBuffer(command_buffer, id, dst.id, 1, &copy_region);

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
