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

export module PipelineShading;

import VkDebug;

export struct PipelineShading
{
    struct Shader
    {
        const char* code = nullptr;
        VkShaderStageFlagBits stage{};
        VkPipelineShaderStageCreateFlags flags = 0;

        const char* Name() const
        {
            switch (stage)
            {
                case VK_SHADER_STAGE_VERTEX_BIT:
                    return "Vertex";
                case VK_SHADER_STAGE_FRAGMENT_BIT:
                    return "Fragment";
                default:
                    return "Unknown";
            }
        }

        bool CreateModule(VkDevice logical_device, VkShaderModule& module) const
        {
            if (code == nullptr)
                return false;

            size_t codeSize = strlen(code);
            std::vector<uint32_t> codeBytes((codeSize + 3) / 4);
            memcpy(codeBytes.data(), code, codeSize);

            VkShaderModuleCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = codeBytes.size() * sizeof(uint32_t);
            createInfo.pCode = codeBytes.data();

            return vkCreateShaderModule(logical_device, &createInfo, VkDebug::Allocation(), &module) == VK_SUCCESS;
        }
    };

    // Shaders
    std::vector<Shader> shaders{};

    // Vertex Input
    std::vector<VkVertexInputBindingDescription> binding_descriptions{};
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};

    // Uniforms
    VkDescriptorPool pool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayout> set_layouts{};

    bool CreateDescriptorPool(VkDevice& logical_device, const std::vector<VkDescriptorPoolSize>& pool_sizes,
                              VkDescriptorPoolCreateFlags flags = 0, uint32_t max_sets = 1)
    {
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = flags;
        poolInfo.maxSets = max_sets;
        poolInfo.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        poolInfo.pPoolSizes = pool_sizes.data();
        return vkCreateDescriptorPool(logical_device, &poolInfo, VkDebug::Allocation(), &pool) == VK_SUCCESS;
    }

    bool CreateDescriptorSetLayout(VkDevice& logical_device,
                                   const std::vector<VkDescriptorSetLayoutBinding>& layout_bindings,
                                   VkDescriptorSetLayoutCreateFlags flags = 0)
    {
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.flags = flags;
        layoutInfo.bindingCount = static_cast<uint32_t>(layout_bindings.size());
        layoutInfo.pBindings = layout_bindings.data();

        set_layouts.push_back(VK_NULL_HANDLE);
        return vkCreateDescriptorSetLayout(logical_device, &layoutInfo, VkDebug::Allocation(), &set_layouts.back()) ==
               VK_SUCCESS;
    }

    void Clear(VkDevice logical_device)
    {
        shaders.clear();
        binding_descriptions.clear();
        attribute_descriptions.clear();
    
        vkDestroyDescriptorPool(logical_device, pool, VkDebug::Allocation());
        pool = VK_NULL_HANDLE;

        for (auto& set_layout : set_layouts)
            vkDestroyDescriptorSetLayout(logical_device, set_layout, VkDebug::Allocation());
        set_layouts.clear();
    }

    bool GetShaderStageCreateInfo(VkDevice logical_device, std::vector<VkPipelineShaderStageCreateInfo>& info) const
    {
        for (const auto& shader : shaders)
        {
            info.push_back({VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, shader.flags, shader.stage,
                            VK_NULL_HANDLE, "main", nullptr});

            if (!shader.CreateModule(logical_device, info.back().module))
                return false;
        }

        return true;
    }

    void FillVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo& vertexInputInfo) const
    {
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = binding_descriptions.data();
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attribute_descriptions.data();
    }
};