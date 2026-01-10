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
#include <MathGeoLib.h>

#include <iostream>
#include <vector>
#include <array>

export module ShadingContext;

import Uniforms;
import Shading;
import PipelineShading;

import Camera;
import Mesh;
import Material;
import ECS;

struct InstancedShading : public Shading
{
    const char* VertexShader() const override
    {
        return R"(
        #version 450
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 v_normal;
        layout(location = 2) in mat4 model;
            
        layout(location = 0) out vec3 f_normal;

        layout(set = 1, binding = 0, row_major) uniform Camera
        {
            mat4 view_proj;
        } cam;
        
        void main() {
            gl_Position = cam.view_proj * (vec4(position, 1.0) * model);
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
        return {{0, sizeof(math::float3),   VK_VERTEX_INPUT_RATE_VERTEX  },  // position
                {1, sizeof(math::float3),   VK_VERTEX_INPUT_RATE_INSTANCE},  // v_normal
                {2, sizeof(math::float4x4), VK_VERTEX_INPUT_RATE_INSTANCE}}; // model
    }
    std::vector<VkVertexInputAttributeDescription> AttributeDescriptions() const override
    {
        return {{0, 0, VK_FORMAT_R32G32B32_SFLOAT,    0}, // position
                {1, 1, VK_FORMAT_R32G32B32_SFLOAT,    0}, // v_normal
                {2, 2, VK_FORMAT_R32G32B32A32_SFLOAT, 0}, // model
                {3, 2, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 4},
                {4, 2, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 8},
                {5, 2, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 12}};
    }
    std::map<uint32_t, DescriptorSetInfo> MapDescriptorSets() const override
    {
        std::map<uint32_t, DescriptorSetInfo> descriptor_sets{};
        descriptor_sets.insert({0, {false, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}}}}); // material
        descriptor_sets.insert({1, { true, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,   VK_SHADER_STAGE_VERTEX_BIT, nullptr}}}}); // cam
        return descriptor_sets;
    }
    std::vector<UniformCreateInfo> GlobalUniforms() const override
    {
        return {UniformCreateInfo::Filled(math::float4x4::identity)};
    }

    bool Load(VkDevice logical_device, VkQueue queue, const VkPhysicalDeviceMemoryProperties* mem_properties,
              VkCommandPool cmd_pool, const Mesh& mesh, const Material& material) override
    {
        Material::Uniform material_uniform = material.ToUniform();

        return AddVertexBuffer(logical_device, mem_properties, queue, cmd_pool, mesh.vertices.get(),
                               mesh.vertex_count * sizeof(float) * 3) &&
               AddVertexBuffer(logical_device, mem_properties, queue, cmd_pool, mesh.normals.get(),
                               mesh.vertex_count * sizeof(float) * 3) &&
               AddIndexBuffer(logical_device, mem_properties, queue, cmd_pool, mesh.indices.get(),
                              mesh.face_count * sizeof(unsigned int) * 3) &&
               local_uniforms.Create(logical_device, mem_properties, {UniformCreateInfo::Filled(material_uniform)}) &&
               AddTransform(logical_device, mem_properties, 4);
    }

    
    bool Update(VkDevice logical_device, float global_time, const std::vector<const void*>& local_uniform_ptrs) override
    {
        if (transforms.empty()) return true;

        int i = -1;
        for (auto& transform : transforms)
        {
            float time = global_time * (1.f + 0.1f * (++i));
            transform.SetTranslatePart(sin(time) * 0.5f, cos(time) * 0.5f, 0.f);
        }

        if (InstanceBuffer().FillData(logical_device, transforms.data()))
        {
            std::cerr << "Failed to Update transforms' Instance Buffer!" << std::endl;
            return false;
        }
        
        return local_uniforms.Update(logical_device, local_uniform_ptrs);
    }



    bool AddTransform(VkDevice logical_device, const VkPhysicalDeviceMemoryProperties* mem_properties,
                      uint32_t to_add = 1)
    {
        if (to_add == 0) return true;

        bool first_time = transforms.empty();
        if (first_time) vertex_buffers.push_back({});

        Buffer& buffer = InstanceBuffer();
        if (!first_time) buffer.Clear(logical_device);

        instance_count = transforms.size() + to_add;
        transforms.resize(instance_count, math::float4x4::identity);
        return buffer.CreateEmpty(logical_device, mem_properties, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                  instance_count * sizeof(math::float4x4));
    }

  protected:

    std::vector<math::float4x4> transforms{};

    inline Buffer& InstanceBuffer() { return vertex_buffers.back(); }
    inline const Buffer& InstanceBuffer() const { return vertex_buffers.back(); }
};

struct MeshShading : public Shading
{
    const char* VertexShader() const override
    {
        return R"(
        #version 450
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 v_normal;

        layout(location = 0) out vec3 f_normal;
        
        layout(set = 1, binding = 0, row_major) uniform Transform
        {
            mat4 model;
        } trs;
        
        layout(set = 2, binding = 0, row_major) uniform Camera
        {
            mat4 view_proj;
        } cam;

        void main() {
            gl_Position = cam.view_proj * trs.model * vec4(position, 1.0);
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
        return {{0, sizeof(math::float3), VK_VERTEX_INPUT_RATE_VERTEX},
                {1, sizeof(math::float3), VK_VERTEX_INPUT_RATE_VERTEX}};
    }
    std::vector<VkVertexInputAttributeDescription> AttributeDescriptions() const override
    {
        return {{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
                {1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0}};
    }
    std::map<uint32_t, DescriptorSetInfo> MapDescriptorSets() const override
    {
        std::map<uint32_t, DescriptorSetInfo> descriptor_sets{};
        descriptor_sets.insert({0, {false, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}}}}); // Material
        descriptor_sets.insert({1, {false, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT,   nullptr}}}}); // Transform
        descriptor_sets.insert({2, { true, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT,   nullptr}}}}); // Camera
        return descriptor_sets;
    }
    std::vector<UniformCreateInfo> GlobalUniforms() const override
    {
        return {UniformCreateInfo::Filled(math::float4x4::identity)};
    }

    bool Load(VkDevice logical_device, VkQueue queue, const VkPhysicalDeviceMemoryProperties* mem_properties,
        VkCommandPool cmd_pool, const Mesh& mesh, const Material& material) override
    {
        Material::Uniform material_uniform = material.ToUniform();
        math::float4x4 model = math::float4x4::identity;

        return AddVertexBuffer(logical_device, mem_properties, queue, cmd_pool, mesh.vertices.get(),
                               mesh.vertex_count * sizeof(float) * 3) &&
               AddVertexBuffer(logical_device, mem_properties, queue, cmd_pool, mesh.normals.get(),
                               mesh.vertex_count * sizeof(float) * 3) &&
               AddIndexBuffer(logical_device, mem_properties, queue, cmd_pool, mesh.indices.get(),
                              mesh.face_count * sizeof(unsigned int) * 3) &&
               local_uniforms.Create(logical_device, mem_properties,
                                     {UniformCreateInfo::Filled(material_uniform), UniformCreateInfo::Filled(model)});
    }

    bool Update(VkDevice logical_device, float global_time, const std::vector<const void*>& local_uniform_ptrs) override
    {
        math::float4x4 model = math::float4x4::identity;
        model.SetTranslatePart({sin(global_time) * 0.5f, cos(global_time) * 0.5f, 0.f });

        return local_uniforms.Update(logical_device, {nullptr, &model});
    }
};

export namespace ShadingContext
{
    enum class Type : char
    {
        Fast,
        Instanced,
        Mesh
    };

    export struct Manager
    {
      private:

        Type type = Type::Fast;
        ShadingCollection<FastShading> fast{};
        ShadingCollection<InstancedShading> instanced{};
        ShadingCollection<MeshShading> mesh{};

      public:

        const PipelineShadingSpecs& ShadingSpecs() const
        {
            switch (type)
            {
                default: return fast.pipeline_specs;
                case Type::Instanced: return instanced.pipeline_specs;
                case Type::Mesh: return mesh.pipeline_specs;
            }
        }

        bool Init(VkDevice logical_device, 
            const VkPhysicalDeviceMemoryProperties* mem_properties, Type _type)
        {
            switch (type = _type)
            {
                case Type::Fast:      return fast.Init(logical_device, mem_properties);
                case Type::Instanced: return instanced.Init(logical_device, mem_properties);
                case Type::Mesh:      return mesh.Init(logical_device, mem_properties);
                default: return false;
            }
        }

        void Clear(VkDevice logical_device)
        {
            fast.Clear(logical_device);
            instanced.Clear(logical_device);
            mesh.Clear(logical_device);
        }

        bool LoadMesh(VkDevice logical_device, VkQueue queue,
                          const VkPhysicalDeviceMemoryProperties* mem_properties, VkCommandPool cmd_pool,
                          const Mesh& mesh_to_load, const Material& material = {})
        {
            switch (type)
            {
                case Type::Fast: return fast.Load(logical_device, queue, mem_properties, cmd_pool, mesh_to_load, material);
                case Type::Instanced: return instanced.Load(logical_device, queue, mem_properties, cmd_pool, mesh_to_load, material);
                case Type::Mesh: return mesh.Load(logical_device, queue, mem_properties, cmd_pool, mesh_to_load, material);
                default: return false;
            }
        }

        bool PreparePool(VkDevice logical_device)
        {
            switch (type)
            {
                case Type::Fast: return fast.PreparePool(logical_device);
                case Type::Instanced: return instanced.PreparePool(logical_device);
                case Type::Mesh: return mesh.PreparePool(logical_device);
                default: return false;
            }
        }

        void Log(const char* name, math::float4x4 mat)
        {
            std::cout << name << ": " << std::endl;
            for (int row = 0; row < 4; row++)
            {
                std::cout << '\t';
                for (int col = 0; col < 4; col++)
                    std::cout << mat.At(row, col) << ", ";
                std::cout << std::endl;
            }
        }

        bool Update(VkDevice logical_device, float global_time, ECS& ecs)
        {
            Camera& cam = ecs.MainCamera();
            //cam.Orbit(0.f, 0.1f, math::float3::zero);
            math::float4x4 view_proj = cam.Projection() * cam.View();

            //UniformType::UCamera camera = {cam.View(), cam.Projection()};
            //UniformType::UCamera camera = {cam.View(), math::float4x4::identity};
            //UniformType::UCamera camera = {cam.View(), cam.frustum.ProjectionMatrix()};

            //Log("WorldMatrix()", cam.frustum.WorldMatrix());
            //Log("ViewMatrix()", cam.frustum.ViewMatrix());
            //Log("ProjectionMatrix()", cam.frustum.ProjectionMatrix());
            //Log("ViewProjMatrix()", cam.frustum.ViewProjMatrix());
            //Log("view", cam.View());
            //Log("proj", cam.Projection());
            //Log("view * proj", cam.View() * cam.Projection());
            //Log("proj * view", view_proj);
            //
            //
            //Log("D3DOrthoProjLH",
            //    math::float4x4::D3DOrthoProjLH(cam.frustum.nearPlaneDistance, cam.frustum.farPlaneDistance,
            //                                   cam.bounds.x, cam.bounds.y));
            //Log("D3DOrthoProjRH",
            //    math::float4x4::D3DOrthoProjRH(cam.frustum.nearPlaneDistance, cam.frustum.farPlaneDistance,
            //                                   cam.bounds.x, cam.bounds.y));
            //Log("D3DPerspProjLH",
            //    math::float4x4::D3DPerspProjLH(cam.frustum.nearPlaneDistance, cam.frustum.farPlaneDistance,
            //                                   cam.bounds.x, cam.bounds.y));
            //Log("D3DPerspProjRH",
            //    math::float4x4::D3DPerspProjRH(cam.frustum.nearPlaneDistance, cam.frustum.farPlaneDistance,
            //                                   cam.bounds.x, cam.bounds.y));
            //Log("OpenGLPerspProjRH",
            //    math::float4x4::OpenGLPerspProjRH(cam.frustum.nearPlaneDistance, cam.frustum.farPlaneDistance,
            //                                   cam.bounds.x, cam.bounds.y));
            //Log("OrthographicProjection",
            //    math::float4x4::OrthographicProjection(cam.frustum.NearPlane()));
            //std::cout << "NearPlane(): " << cam.frustum.NearPlane() << std::endl;
            //
            //std::cout << "Camera:" << std::endl
            //          << "\ttarget_ar: " << static_cast<unsigned short>(cam.target_ar) << std::endl
            //          << "\tar_bounds: " << cam.bounds << std::endl
            //          << "\tfrustum: " << cam.frustum << std::endl << std::endl << std::endl;

            Material::Uniform material{};
            //material.diffuse[0] = (sin(global_time) * 0.5f) + 0.5f; // X-axis
            //material.diffuse[1] = (cos(global_time) * 0.5f) + 0.5f; // Y-axis

            switch (type)
            {
                case Type::Fast:
                    return fast.Update(logical_device, global_time, {&view_proj}, {&material});
                case Type::Instanced: 
                    return instanced.Update(logical_device, global_time, {&view_proj}, {&material});
                default:
                    return mesh.Update(logical_device, global_time, {&view_proj}, {});
            }
            return true;
        }

        void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
        {
            switch (type)
            {
                case Type::Fast: fast.Draw(commandBuffer, pipelineLayout); break;
                case Type::Instanced: instanced.Draw(commandBuffer, pipelineLayout); break;
                case Type::Mesh: mesh.Draw(commandBuffer, pipelineLayout); break;
                default: break;
            }
        }
    };
} // namespace ShadingContext
