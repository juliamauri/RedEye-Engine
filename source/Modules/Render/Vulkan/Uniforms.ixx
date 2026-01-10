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

#include <vector>

export module Uniforms;

import Buffers;
import Utility;

export struct UniformCreateInfo
{
    const void* ptr;
    uint32_t binding = 0;
    VkDeviceSize size = 0;
    VkDeviceSize offset = 0;
    
    template <typename T>
    static UniformCreateInfo Empty(uint32_t binding = 0, VkDeviceSize buffer_offset = 0)
    {
        return {nullptr, binding, sizeof(T), buffer_offset};
    }
    template <typename T>
    static UniformCreateInfo Empty(T& data, uint32_t binding = 0, VkDeviceSize buffer_offset = 0)
    {
        return {nullptr, binding, sizeof(T), buffer_offset};
    }
    template <typename T>
    static UniformCreateInfo Filled(T& data, uint32_t binding = 0, VkDeviceSize buffer_offset = 0)
    {
        return {(void*)&data, binding, sizeof(T), buffer_offset};
    }
};

export struct Uniform
{
    Buffer buffer{};
    uint32_t binding = 0;

    bool Create(VkDevice logical_device, const VkPhysicalDeviceMemoryProperties* mem_properties,
                const UniformCreateInfo& create_info)
    {
        binding = create_info.binding;
        return 
            buffer.CreateEmpty(logical_device, mem_properties, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                create_info.size, create_info.offset) && 
            (create_info.ptr == nullptr || buffer.FillData(logical_device, create_info.ptr));
    }

    void Clear(VkDevice logical_device)
    {
        buffer.Clear(logical_device);
    }

    inline bool Update(VkDevice logical_device, const void* data) const
    {
        return data != nullptr && buffer.FillData(logical_device, data);
    }
};

export struct Uniforms
{
    std::vector<Uniform> uniforms{};

    bool Create(VkDevice logical_device, const VkPhysicalDeviceMemoryProperties* mem_properties,
                const std::vector<UniformCreateInfo>& uniform_infos)
    {
        auto size = uniform_infos.size();
        uniforms.resize(size);

        int iter = -1;
        for (auto& uniform : uniforms)
            if (!uniform.Create(logical_device, mem_properties, uniform_infos[++iter]))
                return false;

        return true;
    }

    bool Update(VkDevice logical_device, const std::vector<const void*>& ptrs) const
    {
        int iter = -1;
        for (const auto& uniform : uniforms)
        {
            const void* ptr = ptrs[++iter];
            if (ptr == nullptr)
                continue;
            if (!uniform.Update(logical_device, ptr))
                return false;
        }
        return true;
    }

    Uniform& At(uint32_t index) { return uniforms.at(index); }
    void Clear(VkDevice logical_device) { CLEAR(uniforms, &Uniform::Clear, logical_device); }
};
