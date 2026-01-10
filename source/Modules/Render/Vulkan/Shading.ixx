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
#include <Math/float3.h>

#include <vector>
#include <map>
#include <type_traits>

export module Shading;

import VkDebug;
import Buffers;
import Uniforms;
import PipelineShading;
import Utility;

import Mesh;
import Material;

inline const VkAllocationCallbacks* Allocation()
{
    return VkDebug::Allocation();
}

export struct DescriptorSetInfo
{
    bool is_global = false;
    std::vector<VkDescriptorSetLayoutBinding> bindings{};
    VkDescriptorSetLayoutCreateFlags flags = 0;
};

export struct Shading
{
    Buffer index_buffer{};
    uint32_t instance_count = 1;
    std::vector<Buffer> vertex_buffers{};

    Uniforms local_uniforms{};
    std::vector<VkDescriptorSet> desciptor_sets{};

    virtual const char* VertexShader() const = 0;
    virtual const char* FragmentShader() const = 0;
    virtual std::vector<ShaderProgram> GetShaders() const
    {
        return 
        {
            {VertexShader(), VK_SHADER_STAGE_VERTEX_BIT},
            {FragmentShader(), VK_SHADER_STAGE_FRAGMENT_BIT}
        };
    }
    virtual std::vector<VkVertexInputBindingDescription> BindingDescriptions() const = 0;
    virtual std::vector<VkVertexInputAttributeDescription> AttributeDescriptions() const = 0;
    virtual std::map<uint32_t, DescriptorSetInfo> MapDescriptorSets() const = 0;
    virtual std::vector<UniformCreateInfo> GlobalUniforms() const { return {}; }

    /* 
    struct VkVertexInputBindingDescription:
        uint32_t binding;
        uint32_t stride;
        VkVertexInputRate inputRate;

    struct VkVertexInputAttributeDescription:
        uint32_t location;
        uint32_t binding;
        VkFormat format;
        uint32_t offset;

    struct DescriptorSetInfo:
        bool is_global = false;
        std::vector<VkDescriptorSetLayoutBinding> bindings{};
        VkDescriptorSetLayoutCreateFlags flags = 0;

    struct UniformCreateInfo:
        const void* ptr;
        uint32_t binding = 0;
        VkDeviceSize size = 0;
        VkDeviceSize offset = 0;
    */

    
    virtual bool Load(VkDevice logical_device, VkQueue queue, const VkPhysicalDeviceMemoryProperties* mem_properties,
                      VkCommandPool cmd_pool, const Mesh& mesh_to_upload, const Material& material) = 0;

    virtual bool Update(VkDevice logical_device, float global_time,
                        const std::vector<const void*>& local_uniform_ptrs) = 0;



    inline bool AddVertexBuffer(VkDevice logical_device, const VkPhysicalDeviceMemoryProperties* mem_properties,
                               VkQueue queue,
                         VkCommandPool cmd_pool, const void* data, VkDeviceSize buffer_size,
                         VkDeviceSize buffer_offset = 0)
    {
        vertex_buffers.push_back({});
        return vertex_buffers.back().CreateFromStagedCopy(logical_device, mem_properties, queue, cmd_pool, data,
                                                          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, buffer_size,
                                                          buffer_offset);
    }

    inline bool AddIndexBuffer(VkDevice logical_device, const VkPhysicalDeviceMemoryProperties* mem_properties,
                               VkQueue queue, VkCommandPool cmd_pool, const void* data, VkDeviceSize buffer_size,
                               VkDeviceSize buffer_offset = 0)
    {
        return index_buffer.CreateFromStagedCopy(logical_device, mem_properties, queue, cmd_pool, data,
                                                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT, buffer_size, buffer_offset);
    }

    void Clear(VkDevice logical_device)
    {
        index_buffer.Clear(logical_device);
        instance_count = 0;
        for (auto& vertex : vertex_buffers)
            vertex.Clear(logical_device);
        local_uniforms.Clear(logical_device);
    }

    void BindVertexBuffers(VkCommandBuffer cmd_buffer, uint32_t first_binding = 0) const
    {
        std::vector<VkBuffer> v_buffers{};
        std::vector<VkDeviceSize> v_offsets{};

        auto vertex_buffer_count = vertex_buffers.size();
        v_buffers.resize(vertex_buffer_count);
        v_offsets.resize(vertex_buffer_count);
        for (unsigned int i = 0; i < vertex_buffer_count; i++)
        {
            const auto& vertex_buffer = vertex_buffers[i];
            v_buffers[i] = vertex_buffer.id;
            v_offsets[i] = vertex_buffer.offset;
        }
        vkCmdBindVertexBuffers(cmd_buffer, first_binding, vertex_buffers.size(), v_buffers.data(), v_offsets.data());
    }

    inline void BindIndexBuffer(VkCommandBuffer cmd_buffer) const
    {
        vkCmdBindIndexBuffer(cmd_buffer, index_buffer.id, index_buffer.offset, VK_INDEX_TYPE_UINT32);
    }

    inline void Draw(VkCommandBuffer cmd_buffer, uint32_t first_index = 0, int32_t vertex_offset = 0,
                     uint32_t first_instance = 0) const
    {
        vkCmdDrawIndexed(cmd_buffer, index_buffer.Count(sizeof(unsigned int)), instance_count, first_index,
                         vertex_offset, first_instance);
    }

    inline void BindDescriptorSets(VkCommandBuffer cmd_buffer, VkPipelineLayout pipeline_layout,
                                   uint32_t first_set = 0) const
    {
        vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, first_set,
                                desciptor_sets.size(), desciptor_sets.data(), 0, nullptr);
    }
};

export template <typename Drawable> struct ShadingCollection
{
    static_assert(std::is_base_of_v<Shading, Drawable>,
                  "ShadingCollection must define a Drawable type that inherits from Shading");

    // Uploaded Geometry
    std::vector<Drawable> drawables{};

    // Uniforms & Specs
    Uniforms global_uniforms{};
    uint32_t local_uniforms_count = 0;
    PipelineShadingSpecs pipeline_specs{};

    // Descriptor Pool
    uint32_t max_sets = 0;
    VkDescriptorPool pool = VK_NULL_HANDLE;
    std::map<uint32_t, DescriptorSetInfo> descriptor_set_infos{};

    bool Init(VkDevice logical_device, const VkPhysicalDeviceMemoryProperties* mem_properties)
    {
        {
            Drawable tmp{};

            // Create PipelineShading
            pipeline_specs = {tmp.GetShaders(), tmp.BindingDescriptions(), tmp.AttributeDescriptions()};

            descriptor_set_infos = tmp.MapDescriptorSets();

            // Create Global Uniforms
            if (!global_uniforms.Create(logical_device, mem_properties, tmp.GlobalUniforms()))
                return false;
        }

        // Create Descriptor Set Layouts
        pipeline_specs.layouts.resize(descriptor_set_infos.size(), VK_NULL_HANDLE);
        for (const auto& [set, info] : descriptor_set_infos)
        {
            local_uniforms_count += !info.is_global;

            VkDescriptorSetLayoutCreateInfo layout_info{};
            layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layout_info.flags = info.flags;
            layout_info.bindingCount = static_cast<uint32_t>(info.bindings.size());
            layout_info.pBindings = info.bindings.data();
            if (vkCreateDescriptorSetLayout(logical_device, &layout_info, Allocation(), &pipeline_specs.layouts.at(set)) != VK_SUCCESS)
                return false;
        }

        return true;
    }

    void Clear(VkDevice logical_device)
    {
        CLEAR(drawables, &Drawable::Clear, logical_device);
        global_uniforms.Clear(logical_device);
        pipeline_specs.Clear(logical_device);

        if (pool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(logical_device, pool, Allocation());
            pool = VK_NULL_HANDLE;
        }
    }

    bool Load(VkDevice logical_device, VkQueue queue, const VkPhysicalDeviceMemoryProperties* mem_properties,
                   VkCommandPool cmd_pool, const Mesh& mesh, const Material& material = {})
    {
        drawables.push_back({});
        return drawables.back().Load(logical_device, queue, mem_properties, cmd_pool, mesh, material);
    }

    bool Update(VkDevice logical_device, float global_time,
        const std::vector<const void*>& global_uniform_ptrs,
        const std::vector<const void*>&  local_uniform_ptrs)
    {
        if (drawables.empty()) return true;
        if (!global_uniforms.Update(logical_device, global_uniform_ptrs)) return false;
        for (auto& drawable : drawables)
            if (!drawable.Update(logical_device, global_time, local_uniform_ptrs))
                return false;
        return true;
    }

    bool PreparePool(VkDevice logical_device, VkDescriptorPoolCreateFlags pool_flags = 0)
    {
        uint32_t total_sets = global_uniforms.uniforms.size() + (local_uniforms_count * drawables.size());
        if (pool != VK_NULL_HANDLE && max_sets >= total_sets)
            return true;

        // Clear Pool
        if (pool != VK_NULL_HANDLE)
            vkDestroyDescriptorPool(logical_device, pool, Allocation());

        // Create Pool & Allocate Descriptors
        max_sets = total_sets;
        std::vector<VkDescriptorPoolSize> pool_sizes = {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, max_sets}};
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = pool_flags;
        poolInfo.maxSets = max_sets;
        poolInfo.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        poolInfo.pPoolSizes = pool_sizes.data();
        if (vkCreateDescriptorPool(logical_device, &poolInfo, Allocation(), &pool) != VK_SUCCESS ||
            pool == VK_NULL_HANDLE)
            return false;

        uint32_t local_sets = descriptor_set_infos.size();
        for (auto& drawable : drawables)
            drawable.desciptor_sets.resize(local_sets, VK_NULL_HANDLE);

        std::vector<VkDescriptorBufferInfo> buffer_infos{};
        std::vector<VkWriteDescriptorSet> to_write{};
        buffer_infos.resize(max_sets);
        to_write.resize(max_sets);

        int local_iter = -1;
        int global_iter = -1;
        int buffer_iter = -1;

        for (const auto& [set, info] : descriptor_set_infos)
        {
            if (info.is_global)
            {
                // Create Descriptor Set
                VkDescriptorSetAllocateInfo alloc_info{};
                alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                alloc_info.descriptorPool = pool;
                alloc_info.descriptorSetCount = 1;
                alloc_info.pSetLayouts = &pipeline_specs.layouts.at(set);

                VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
                if (vkAllocateDescriptorSets(logical_device, &alloc_info, &descriptor_set) != VK_SUCCESS ||
                    descriptor_set == VK_NULL_HANDLE)
                    return false;

                // Assign
                for (auto& drawable : drawables)
                    drawable.desciptor_sets.at(set) = descriptor_set;

                // Get Uniform Buffer
                buffer_iter++;
                const Uniform& uniform = global_uniforms.uniforms.at(++global_iter);
                VkDescriptorBufferInfo& buffer_info = buffer_infos.at(buffer_iter);
                buffer_info = uniform.buffer.DescriptorBufferInfo();

                // Update Descriptor Set
                VkWriteDescriptorSet& write = to_write.at(buffer_iter);
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptor_set;
                write.dstBinding = uniform.binding;
                write.dstArrayElement = 0;
                write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write.descriptorCount = 1;
                write.pBufferInfo = &buffer_info;
                continue;
            }

            // Create Descriptor Set for each drawable
            VkDescriptorSetAllocateInfo alloc_info{};
            alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            alloc_info.descriptorPool = pool;
            alloc_info.descriptorSetCount = drawables.size();
            alloc_info.pSetLayouts = &pipeline_specs.layouts.at(set);

            std::vector<VkDescriptorSet> descriptor_sets{};
            descriptor_sets.resize(alloc_info.descriptorSetCount, VK_NULL_HANDLE);
            if (vkAllocateDescriptorSets(logical_device, &alloc_info, descriptor_sets.data()) != VK_SUCCESS)
                return false;

            int iter = -1;
            local_iter++;
            for (auto& drawable : drawables)
            {
                // Assign
                VkDescriptorSet descriptor_set = descriptor_sets[++iter];
                drawable.desciptor_sets.at(set) = descriptor_set;

                // Get Uniform Buffer
                buffer_iter++;
                const Uniform& uniform = drawable.local_uniforms.uniforms.at(local_iter);
                VkDescriptorBufferInfo& buffer_info = buffer_infos.at(buffer_iter);
                buffer_info = uniform.buffer.DescriptorBufferInfo();

                // Update Descriptor Set
                VkWriteDescriptorSet& write = to_write.at(buffer_iter);
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptor_set;
                write.dstBinding = uniform.binding;
                write.dstArrayElement = 0;
                write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write.descriptorCount = 1;
                write.pBufferInfo = &buffer_info;
            }
        }

        vkUpdateDescriptorSets(logical_device, to_write.size(), to_write.data(), 0, nullptr);
        return true;
    }

    void Draw(VkCommandBuffer cmd_buffer, VkPipelineLayout pipeline_layout) const
    {
        for (const auto& drawable : drawables)
        {
            drawable.BindDescriptorSets(cmd_buffer, pipeline_layout);
            drawable.BindVertexBuffers(cmd_buffer);
            drawable.BindIndexBuffer(cmd_buffer);
            drawable.Draw(cmd_buffer);
        }
    }
};

export struct FastShading : public Shading
{
    struct Transfoms
    {
        float model[16] = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
        float view[16] = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
        float proj[16] = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
    };

    const char* VertexShader() const override
    {
        return R"(
                #version 450
                layout(location = 0) in vec3 position;
                layout(location = 1) in vec3 v_normal;
                
                layout(location = 0) out vec3 f_normal;
                
                layout(set = 1, binding = 0) uniform AllTransforms {
                    mat4 model;
                    mat4 view;
                    mat4 proj;
                } trs;
                
                void main() {
                    gl_Position = trs.proj * trs.view * trs.model * vec4(position, 1.0);
                    f_normal = v_normal;
                }
            )";
    }
    const char* FragmentShader() const override
    {
        return R"(
                #version 450
                layout(location = 0) in vec3 f_normal;
    
                layout(location = 0) out vec4 color;
    
                layout(set = 0, binding = 0) uniform Material {
                    vec3 diffuse;
                } material;
    
                void main() {
                    color = vec4(material.diffuse, 1.0);
                }
            )";
    }
    std::vector<VkVertexInputBindingDescription> BindingDescriptions() const override
    {
        return {{0, sizeof(float) * 3, VK_VERTEX_INPUT_RATE_VERTEX},  // position
                {1, sizeof(float) * 3, VK_VERTEX_INPUT_RATE_VERTEX}}; // v_normal
    }
    std::vector<VkVertexInputAttributeDescription> AttributeDescriptions() const override
    {
        return {{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},  // position
                {1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0}}; // v_normal
    }
    std::map<uint32_t, DescriptorSetInfo> MapDescriptorSets() const override
    {
        std::map<uint32_t, DescriptorSetInfo> descriptor_sets{};
        descriptor_sets.insert(
            {0, {false, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}}}});
        descriptor_sets.insert(
            {1, {false, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}}}});
        return descriptor_sets;
    }

    bool Load(VkDevice logical_device, VkQueue queue, const VkPhysicalDeviceMemoryProperties* mem_properties,
        VkCommandPool cmd_pool, const Mesh& mesh, const Material& material) override
    {
        Material::Uniform material_uniform = material.ToUniform();
        FastShading::Transfoms transforms{};

        return AddVertexBuffer(logical_device, mem_properties, queue, cmd_pool, mesh.vertices.get(),
                               mesh.vertex_count * sizeof(float) * 3) &&
               AddVertexBuffer(logical_device, mem_properties, queue, cmd_pool, mesh.normals.get(),
                               mesh.vertex_count * sizeof(float) * 3) &&
               AddIndexBuffer(logical_device, mem_properties, queue, cmd_pool, mesh.indices.get(),
                              mesh.face_count * sizeof(unsigned int) * 3) &&
               local_uniforms.Create(logical_device, mem_properties,
                   {UniformCreateInfo::Filled(material_uniform), UniformCreateInfo::Filled(transforms)});

    }

    bool Update(VkDevice logical_device, float global_time, const std::vector<const void*>& local_uniform_ptrs) override
    {
        Transfoms transform_matrices{};
        transform_matrices.model[12] = sin(global_time) * 0.5f; // X-axis
        transform_matrices.model[13] = cos(global_time) * 0.5f; // Y-axis

        return local_uniforms.Update(logical_device, {local_uniform_ptrs[0], &transform_matrices});
    }
};