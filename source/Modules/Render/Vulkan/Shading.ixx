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
import Uniforms;
import PipelineShading;

export struct Shading
{
    struct PosColor
    {
        float pos[3];
        float color[3];
    };
    
    // Buffers
    Buffer vertex{};
    Buffer index{};
    std::vector<Uniform> uniforms{};

    // Geometry Data
    uint32_t vertex_count = 0;
    uint32_t index_count = 0;

    // Binding Cached Dta
    uint32_t instance_count = 0;
    std::vector<VkDescriptorSet> descriptor_sets{};

    template <typename VertexInputType, typename IndexInputType>
    bool Create(VkDevice logical_device,
                VkQueue queue,
                VkPhysicalDeviceMemoryProperties mem_properties,
                VkCommandPool commandPool, const PipelineShading& pipeline_shading,
                const std::vector<VertexInputType>& vertices,
                const std::vector<IndexInputType>& indices,
                const std::vector<UniformCreateInfo> uniform_infos)
    {
        if (!SetVertexBuffer(logical_device, queue, mem_properties, commandPool, vertices) ||
            !SetIndexBuffer(logical_device, queue, mem_properties, commandPool, indices))
            return false;

        for (const auto& uniform_info : uniform_infos)
            if (!AddUniform(logical_device, mem_properties, pipeline_shading, uniform_info))
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
        vkCmdDrawIndexed(commandBuffer, index_count, instance_count, 0, 0, 0);
    }

    virtual const char* VertexShader() const = 0;
    virtual const char* FragmentShader() const = 0;
    virtual std::vector<PipelineShading::Shader> GetShaders() const
    {
        return {{VertexShader(), VK_SHADER_STAGE_VERTEX_BIT}, 
                {FragmentShader(), VK_SHADER_STAGE_FRAGMENT_BIT}};
    }
    virtual std::vector<VkVertexInputBindingDescription> BindingDescriptions() const = 0;
    virtual std::vector<VkVertexInputAttributeDescription> AttributeDescriptions() const = 0;
    virtual std::vector<VkDescriptorPoolSize> DescriptorPoolSizes() const = 0;
    virtual std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayoutBindings() const = 0;
    
    bool SetupPipelineShading(VkDevice logical_device, PipelineShading& shading) const
    {
        shading.shaders = GetShaders();
        shading.binding_descriptions = BindingDescriptions();
        shading.attribute_descriptions = AttributeDescriptions();
        return shading.CreateDescriptorPool(logical_device, DescriptorPoolSizes()) &&
               shading.CreateDescriptorSetLayout(logical_device, DescriptorSetLayoutBindings());
    }

    //bool CreateDescriptorSetLayout(VkDevice& logical_device, VkDescriptorSetLayout& descriptor_set_layout) const
    //{
    //    std::vector<VkDescriptorSetLayoutBinding> layout_bindings{};
    //    DescriptorSetLayoutBindings(layout_bindings);
    //
    //    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    //    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    //    layoutInfo.bindingCount = static_cast<uint32_t>(layout_bindings.size());
    //    layoutInfo.pBindings = layout_bindings.data();
    //
    //    return vkCreateDescriptorSetLayout(logical_device, &layoutInfo, VkDebug::Allocation(), &descriptor_set_layout) ==
    //           VK_SUCCESS;
    //}
    //bool CreateDescriptorPool(VkDevice& logical_device, VkDescriptorPool& descriptorPool) const
    //{
    //    std::vector<VkDescriptorPoolSize> pool_sizes{};
    //    DescriptorPoolSizes(pool_sizes);
    //
    //    VkDescriptorPoolCreateInfo poolInfo{};
    //    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    //    poolInfo.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    //    poolInfo.pPoolSizes = pool_sizes.data();
    //    poolInfo.maxSets = 1;
    //
    //    return vkCreateDescriptorPool(logical_device, &poolInfo, VkDebug::Allocation(), &descriptorPool) == VK_SUCCESS;
    //}

  protected:

    template <typename VertexInputType>
    bool SetVertexBuffer(VkDevice logical_device, VkQueue queue, VkPhysicalDeviceMemoryProperties mem_properties,
                         VkCommandPool commandPool, const std::vector<VertexInputType>& vertices,
                         VkDeviceSize buffer_offset = 0)
    {
        vertex_count = static_cast<uint32_t>(vertices.size());
        return vertex.CreateFromStagedCopy(logical_device, queue, mem_properties, commandPool,
                                           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertices, buffer_offset);
    }
    template <typename IndexInputType>
    bool SetIndexBuffer(VkDevice logical_device, VkQueue queue, VkPhysicalDeviceMemoryProperties mem_properties,
                        VkCommandPool commandPool, const std::vector<IndexInputType>& indices,
                        VkDeviceSize buffer_offset = 0)
    {
        index_count = static_cast<uint32_t>(indices.size());
        return index.CreateFromStagedCopy(logical_device, queue, mem_properties, commandPool,
                                          VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices, buffer_offset);
    }

    bool AddUniform(VkDevice logical_device, VkPhysicalDeviceMemoryProperties mem_properties,
                    const PipelineShading& pipeline_shading, UniformCreateInfo uniform_data)
    {
        uniforms.push_back({});
        descriptor_sets.push_back({});
        return uniforms.back().Create(logical_device, mem_properties, 
                                      pipeline_shading.pool, pipeline_shading.set_layouts,
                                      descriptor_sets.back(), uniform_data);
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
    std::vector<VkVertexInputBindingDescription> BindingDescriptions() const override
    {
        return {{0, sizeof(PosColor), VK_VERTEX_INPUT_RATE_VERTEX}};
    }
    std::vector<VkVertexInputAttributeDescription> AttributeDescriptions() const override
    {
        return {{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(PosColor, pos)},
                {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(PosColor, color)}};
    }
    std::vector<VkDescriptorPoolSize> DescriptorPoolSizes() const override
    {
        return {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}};
    }
    std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayoutBindings() const override
    {
        return {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}};
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

    std::vector<VkVertexInputBindingDescription> BindingDescriptions() const override
    {
        return {{0, sizeof(PosColor), VK_VERTEX_INPUT_RATE_VERTEX}, // PosColor
                {1, sizeof(Transform), VK_VERTEX_INPUT_RATE_INSTANCE}}; // Transform
    }
    std::vector<VkVertexInputAttributeDescription> AttributeDescriptions() const override
    {
        return {{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(PosColor, pos)},       // Position
                {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(PosColor, color)},     // Color
                {2, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Transform, model)}, // Model
                {3, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Transform, model) + sizeof(float) * 4},
                {4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Transform, model) + sizeof(float) * 8},
                {5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Transform, model) + sizeof(float) * 12}};
    }
    std::vector<VkDescriptorPoolSize> DescriptorPoolSizes() const override
    {
        return {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}};
    }
    std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayoutBindings() const override
    {
        return {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}};
    }

    bool AddTransform(VkDevice logical_device, VkPhysicalDeviceMemoryProperties mem_properties, uint32_t to_add = 1)
    {
        if (to_add == 0)
            return true;
        if (!transforms.empty())
            instance.Clear(logical_device);
        transforms.resize(transforms.size() + to_add);

        return instance.Create(logical_device, mem_properties, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                               static_cast<VkDeviceSize>(transforms.size()) * sizeof(float) * 16) &&
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
};
