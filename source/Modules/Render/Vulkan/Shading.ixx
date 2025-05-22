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

export module Shading;

import VkDebug;
import Buffers;

struct Shader
{
    const char* code = nullptr;
    VkShaderStageFlagBits stage{};

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

export struct Shading
{
    virtual const char* VertexShader() const = 0;
    virtual const char* FragmentShader() const = 0;
    bool GetStageCreateInfo(VkDevice logical_device, std::vector<VkPipelineShaderStageCreateInfo>& info) const
    {
        std::vector<Shader> shaders{};
        GetShaders(shaders);
        for (const auto& shader : shaders)
        {
            size_t codeSize = strlen(shader.code);
            std::vector<uint32_t> codeBytes((codeSize + 3) / 4);
            memcpy(codeBytes.data(), shader.code, codeSize);

            VkShaderModuleCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = codeBytes.size() * sizeof(uint32_t);
            createInfo.pCode = codeBytes.data();

            info.push_back({VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, shader.stage,
                            VK_NULL_HANDLE, "main", nullptr});
            if (vkCreateShaderModule(logical_device, &createInfo, VkDebug::Allocation(), &info.back().module) !=
                VK_SUCCESS)
                return false;
        }

        return true;
    }

    struct PosColor
    {
        float pos[3];
        float color[3];
    };

    struct Uniform
    {
        void* data = nullptr;
        Buffer buffer{};

        struct CreateInfo
        {
            void* data = nullptr;
            uint32_t binding = 0;
            VkDeviceSize size = 0;

            template <typename T> static CreateInfo From(T& data, uint32_t binding = 0)
            {
                return {&data, binding, sizeof(T)};
            }
        };

        bool Create(const VkDevice logical_device, VkPhysicalDeviceMemoryProperties mem_properties,
                    VkDescriptorPool& descriptorPool, VkDescriptorSetLayout& descriptorSetLayout,
                    VkDescriptorSet& descriptor_set, CreateInfo create_info)
        {
            // Create Uniform Buffer
            if (!buffer.Create(logical_device, mem_properties, create_info.size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) ||
                !buffer.FillData(logical_device, data = create_info.data))
                return false;

            // Create Descriptor Set
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &descriptorSetLayout;

            if (vkAllocateDescriptorSets(logical_device, &allocInfo, &descriptor_set) != VK_SUCCESS)
                return false;

            // Update Descriptor Set
            VkDescriptorBufferInfo bufferInfo = {buffer.id, 0, buffer.buffer_size};
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
    
    // Buffers
    Buffer vertex{};
    Buffer index{};
    std::vector<Uniform> uniforms{};

    // Cached Bindings
    uint32_t instance_count = 0;
    std::vector<VkDescriptorSet> descriptor_sets{};

    template <typename VertexInputType, typename IndexInputType>
    bool Create(VkDevice logical_device,
                VkQueue queue,
                VkPhysicalDeviceMemoryProperties mem_properties,
                VkCommandPool commandPool,
                VkDescriptorPool& descriptorPool,
                VkDescriptorSetLayout& descriptorSetLayout,
                const std::vector<VertexInputType>& vertices,
                const std::vector<IndexInputType>& indices,
                const std::vector<Uniform::CreateInfo> uniform_infos)
    {
        if (!SetVertexBuffer(logical_device, queue, mem_properties, commandPool, vertices) ||
            !SetIndexBuffer(logical_device, queue, mem_properties, commandPool, indices))
            return false;

        for (const auto& uniform_info : uniform_infos)
            if (!AddUniform(logical_device, mem_properties, descriptorPool, descriptorSetLayout, uniform_info))
                return false;

        return true;
    }

    void Clear(VkDevice logical_device)
    {
        ClearDerivates(logical_device);
        descriptor_sets.clear();
        vertex.Clear(logical_device);
        index.Clear(logical_device);
        for (auto& uniform : uniforms)
            uniform.buffer.Clear(logical_device);
    }

    bool UpdateUniforms(VkDevice logical_device) const
    {
        for (const auto& uniform : uniforms)
            if (!uniform.Update(logical_device))
                return false;
        return true;
    }

    void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
    {
        uint32_t instance_count = InstanceCount();
        if (instance_count == 0) return;

        // Bind Uniforms' Descriptor Sets
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0,
                                descriptor_sets.size(), descriptor_sets.data(), 0, nullptr);

        std::vector<VkBuffer> vertex_buffers = {vertex.id};
        std::vector<VkDeviceSize> offsets = {0};
        GetVertexBufferBindings(vertex_buffers, offsets);

        // Bind Vertex Input Buffers
        vkCmdBindVertexBuffers(commandBuffer, 0, vertex_buffers.size(), vertex_buffers.data(), offsets.data());
        vkCmdBindIndexBuffer(commandBuffer, index.id, 0, VK_INDEX_TYPE_UINT32);

        // Draw
        vkCmdDrawIndexed(commandBuffer, index.element_count, instance_count, 0, 0, 0);
    }

    virtual void BindingDescriptions(std::vector<VkVertexInputBindingDescription>& binding_descriptions) const = 0;
    virtual void AttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& attr_descriptions) const = 0;

    bool CreateDescriptorSetLayout(VkDevice& logical_device, VkDescriptorSetLayout& descriptorSetLayout) const
    {
        std::vector<VkDescriptorSetLayoutBinding> layout_bindings{};
        DescriptorSetLayoutBindings(layout_bindings);

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layout_bindings.size());
        layoutInfo.pBindings = layout_bindings.data();

        return vkCreateDescriptorSetLayout(logical_device, &layoutInfo, VkDebug::Allocation(), &descriptorSetLayout) ==
               VK_SUCCESS;
    }
    bool CreateDescriptorPool(VkDevice& logical_device, VkDescriptorPool& descriptorPool) const
    {
        std::vector<VkDescriptorPoolSize> pool_sizes{};
        DescriptorPoolSizes(pool_sizes);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        poolInfo.pPoolSizes = pool_sizes.data();
        poolInfo.maxSets = 1;

        return vkCreateDescriptorPool(logical_device, &poolInfo, VkDebug::Allocation(), &descriptorPool) == VK_SUCCESS;
    }

  protected:
    template <typename VertexInputType>
    bool SetVertexBuffer(VkDevice logical_device, VkQueue queue, VkPhysicalDeviceMemoryProperties mem_properties,
                         VkCommandPool commandPool, const std::vector<VertexInputType>& vertices)
    {
        return vertex.CreateFromStagedCopy(logical_device, queue, mem_properties, commandPool,
                                           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertices);
    }
    template <typename IndexInputType>
    bool SetIndexBuffer(VkDevice logical_device, VkQueue queue, VkPhysicalDeviceMemoryProperties mem_properties,
                        VkCommandPool commandPool, const std::vector<IndexInputType>& indices)
    {
        return index.CreateFromStagedCopy(logical_device, queue, mem_properties, commandPool,
                                          VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices);
    }

    bool AddUniform(VkDevice logical_device, VkPhysicalDeviceMemoryProperties mem_properties,
                    VkDescriptorPool& descriptorPool, VkDescriptorSetLayout& descriptorSetLayout,
                    Uniform::CreateInfo uniform_data)
    {
        uniforms.push_back({});
        descriptor_sets.push_back({});
        return uniforms.back().Create(logical_device, mem_properties, descriptorPool, descriptorSetLayout,
                                      descriptor_sets.back(), uniform_data);
    }

    virtual void GetShaders(std::vector<Shader>& shaders) const
    {
        shaders.push_back({VertexShader(), VK_SHADER_STAGE_VERTEX_BIT});
        shaders.push_back({FragmentShader(), VK_SHADER_STAGE_FRAGMENT_BIT});
    }
    virtual void ClearDerivates(VkDevice logical_device)
    {
    }

    virtual uint32_t InstanceCount() const
    {
        return 1;
    }
    virtual void GetVertexBufferBindings(std::vector<VkBuffer>& vertex_buffers,
                                         std::vector<VkDeviceSize>& offsets) const
    {
    }

    virtual void DescriptorSetLayoutBindings(std::vector<VkDescriptorSetLayoutBinding>& layout_bindings) const = 0;
    virtual void DescriptorPoolSizes(std::vector<VkDescriptorPoolSize>& pool_sizes) const = 0;
};

export struct FastShading : public Shading
{
    const char* VertexShader() const override
    {
        return R"(
            #version 450
            layout(location = 0) in vec3 inPosition;
            layout(location = 1) in vec3 inColor;
            
            layout(location = 0) out vec3 fragColor;
            
            layout(set = 0, binding = 0) uniform TransfomMatrices {
                mat4 model;
                mat4 view;
                mat4 proj;
            } ubo;
            
            void main() {
                gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
                fragColor = inColor;
            }
        )";
    }
    const char* FragmentShader() const override
    {
        return R"(
            #version 450
            layout(location = 0) in vec3 fragColor;
            layout(location = 0) out vec4 outColor;

            void main() {
                outColor = vec4(fragColor, 1.0);
            }
        )";
    }

    struct TransfomMatrices
    {
        float model[16];
        float view[16];
        float proj[16];
    };

    void BindingDescriptions(std::vector<VkVertexInputBindingDescription>& out) const override
    {
        out.push_back({0, sizeof(float) * 6, VK_VERTEX_INPUT_RATE_VERTEX});
    }
    void AttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& out) const override
    {
        out.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(PosColor, pos)});   // Position
        out.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(PosColor, color)}); // Color
    }

  protected:

    void DescriptorSetLayoutBindings(std::vector<VkDescriptorSetLayoutBinding>& out) const override
    {
        out.push_back({0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr});
    }
    void DescriptorPoolSizes(std::vector<VkDescriptorPoolSize>& out) const override
    {
        out.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1});
    }
};

export struct InstancedShading : public Shading
{
    const char* VertexShader() const override
    {
        return R"(
        #version 450
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 color;
        layout(location = 2) in mat4 model;

        layout(location = 0) out vec3 fragColor;

        layout(set = 0, binding = 0) uniform ProjView
        {
            mat4 proj_view;
        } ubo;
        
        void main() {
            gl_Position = model * vec4(position, 1.0);
            fragColor = color;
        }
    )";
    }
    const char* FragmentShader() const override
    {
        return R"(
        #version 450
        layout(location = 0) in vec3 fragColor;
        layout(location = 0) out vec4 outColor;
        void main() {
            outColor = vec4(fragColor, 1.0);
        }
    )";
    }

    struct Transform
    {
        float model[16] = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
    };

    struct ProjectionView
    {
        float proj_view[16];
    };

    Buffer instance{};
    std::vector<Transform> transforms{};

    void BindingDescriptions(std::vector<VkVertexInputBindingDescription>& out) const override
    {
        out.push_back({0, sizeof(PosColor), VK_VERTEX_INPUT_RATE_VERTEX});    // PosColor
        out.push_back({1, sizeof(Transform), VK_VERTEX_INPUT_RATE_INSTANCE}); // Transform
    }
    void AttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& out) const override
    {
        out.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(PosColor, pos)});       // Position
        out.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(PosColor, color)});     // Color
        out.push_back({2, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Transform, model)}); // Model
        out.push_back({3, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Transform, model) + sizeof(float) * 4});
        out.push_back({4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Transform, model) + sizeof(float) * 8});
        out.push_back({5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Transform, model) + sizeof(float) * 12});
    }

    bool AddTransform(VkDevice logical_device, VkPhysicalDeviceMemoryProperties mem_properties, uint32_t to_add = 1)
    {
        if (to_add == 0)
            return true;
        if (!transforms.empty())
            instance.Clear(logical_device);
        transforms.resize(transforms.size() + to_add);

        return instance.Create(logical_device, mem_properties,
                               static_cast<VkDeviceSize>(transforms.size()) * sizeof(float) * 16,
                               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) &&
               UpdateTransforms(logical_device);
    }

    bool UpdateTransforms(VkDevice logical_device) const
    {
        return transforms.empty() || instance.FillData(logical_device, transforms.data());
    }

  protected:
    void ClearDerivates(VkDevice logical_device) override
    {
        instance.Clear(logical_device);
        transforms.clear();
    }

    uint32_t InstanceCount() const override
    {
        return transforms.size();
    }
    void GetVertexBufferBindings(std::vector<VkBuffer>& vertex_buffers,
                                 std::vector<VkDeviceSize>& offsets) const override
    {
        vertex_buffers.push_back(instance.id);
        offsets.push_back(0);
    }

    void DescriptorSetLayoutBindings(std::vector<VkDescriptorSetLayoutBinding>& out) const override
    {
        out.push_back({0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr});
    }
    void DescriptorPoolSizes(std::vector<VkDescriptorPoolSize>& out) const override
    {
        out.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1});
    }
};
