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

#include <SDL2/SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <set>
#include <vector>
#include <map>

export module Vulkan;

import VkDebug;
import VkSDLWindow;
import LayerProperties;
import Surface;
import Device;
import Uniforms;
import Shading;
import PipelineShading;
import GraphicPipeline;

export namespace RE
{
    namespace Vulkan
    {
        bool Init()
        {
            std::cout << "Loading default Vulkan library." << std::endl;
            if (SDL_Vulkan_LoadLibrary(nullptr) != 0)
            {
                std::cerr << "Failed to load default Vulkan library: " << SDL_GetError() << std::endl;
                return false;
            }

            // TODO: Setup Allocation Callbacks for memory management
            VkDebug::SetAllocation(nullptr);

            return LayerProperties::RetrieveAvailable();
        }

        struct Context
        {
            VkInstance instance = VK_NULL_HANDLE;

            Surface surface{};
            LogicalDevice device{};
            GraphicPipeline pipeline{};

            struct ShadingContextDemo
            {
              public:
                enum class Type : char
                {
                    Fast,
                    Instanced
                };

                PipelineShading shading{};

              private:
                Type type = Type::Fast;

                // Fast Shading
                std::vector<FastShading> fast_drawables{};
                FastShading::TransfomMatrices transform_matrices{};

                // Instanced Shading
                std::vector<InstancedShading> instanced_drawables{};
                InstancedShading::ProjectionView proj_view{};

              public:
                bool Init(VkDevice logical_device, Type _type)
                {
                    type = _type;

                    // Fast
                    fast_drawables.push_back({});
                    transform_matrices = {
                        {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f},
                        {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f},
                        {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f}};

                    // Instanced
                    instanced_drawables.push_back({});
                    proj_view = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};

                    return GerFirst()->SetupPipelineShading(logical_device, shading);
                }

                void Clear(VkDevice logical_device)
                {
                    for (auto& drawable : fast_drawables)
                        drawable.Clear(logical_device);
                    for (auto& drawable : instanced_drawables)
                        drawable.Clear(logical_device);
                    fast_drawables.clear();
                    instanced_drawables.clear();
                    transform_matrices = {};
                    proj_view = {};
                }

                template <typename VertexInputType, typename IndexInputType>
                bool LoadGeometry(VkDevice logical_device, VkQueue queue,
                                  VkPhysicalDeviceMemoryProperties mem_properties, VkCommandPool cmd_pool,
                                  const std::vector<VertexInputType>& vertices,
                                  const std::vector<IndexInputType>& indices)
                {
                    if (!GerFirst()->Create(logical_device, queue, mem_properties, cmd_pool, shading, 
                                            vertices, indices, GetUniformCreateInfos()))
                        return false;

                    switch (type)
                    {
                        case Type::Instanced:
                            return instanced_drawables[0].AddTransform(logical_device, mem_properties, 4);
                        default:
                            return true;
                    }
                }

                bool Update(VkDevice logical_device, float global_time)
                {
                    switch (type)
                    {
                        case Type::Instanced:
                        {
                            int i = -1;
                            for (auto& drawable : instanced_drawables)
                            {
                                for (auto& transform : drawable.transforms)
                                {
                                    float time = global_time * (1.f + 0.1f * (++i));
                                    transform.model[12] = sin(time) * 0.5f; // X-axis
                                    transform.model[13] = cos(time) * 0.5f; // Y-axis
                                }

                                if (!drawable.UpdateTransforms(logical_device))
                                {
                                    std::cerr << "Failed to Update Transforms Buffer for Instanced Shading!"
                                              << std::endl;
                                    return false;
                                }
                            }
                            break;
                        }
                        default:
                            break;
                    }
                    return true;
                }

                bool UpdateUniforms(VkDevice logical_device, float time) const
                {
                    switch (type)
                    {
                        case Type::Fast:
                            for (auto& drawable : fast_drawables)
                                if (!drawable.UpdateUniforms(logical_device))
                                    return false;
                            return true;
                        case Type::Instanced:
                            for (auto& drawable : instanced_drawables)
                                if (!drawable.UpdateUniforms(logical_device))
                                    return false;
                            return true;

                        default:
                            return true;
                    }
                }

                void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
                {
                    switch (type)
                    {
                        case Type::Fast:
                            for (auto& drawable : fast_drawables)
                                drawable.Draw(commandBuffer, pipelineLayout);
                            break;
                        case Type::Instanced:
                            for (auto& drawable : instanced_drawables)
                                drawable.Draw(commandBuffer, pipelineLayout);
                            break;
                        default:
                            break;
                    }
                }

                Shading* GerFirst()
                {
                    switch (type)
                    {
                        case Type::Fast:
                            return fast_drawables.data();
                        case Type::Instanced:
                            return instanced_drawables.data();
                        default:
                            return nullptr;
                    }
                }

                const Shading* GerFirst() const
                {
                    switch (type)
                    {
                        case Type::Fast:
                            return fast_drawables.data();
                        case Type::Instanced:
                            return instanced_drawables.data();
                        default:
                            return nullptr;
                    }
                }

                std::vector<UniformCreateInfo> GetUniformCreateInfos()
                {
                    switch (type)
                    {
                        case Type::Fast:
                            return {UniformCreateInfo::From(transform_matrices)};
                        case Type::Instanced:
                            return {UniformCreateInfo::From(proj_view)};
                        default:
                            return {};
                    }
                }
            } demo{};

            bool Create(SDL_Window* window, int w, int h)
            {
                if (!CreateInstance(window))
                    return false;

                if (!surface.Create(window, instance, {static_cast<uint32_t>(w), static_cast<uint32_t>(h)}) ||
                    !device.Create(instance, surface) || 
                    !surface.UpdateCapabilities(device.physical_device) ||
                    !demo.Init(device.logical_device, ShadingContextDemo::Type::Instanced) ||
                    !pipeline.Create(device.logical_device, demo.shading, surface,
                                     device.graphics_family, device.present_family))
                {
                    Delete();
                    return false;
                }

                std::cout << "Graphic Pipeline created. Creating triangle for Demo..." << std::endl;

                std::vector<uint32_t> indices = {0, 1, 2};
                std::vector<Shading::PosColor> vertices = {{{.0f, -.5f, 0.f}, {1.f, 0.f, 0.f}},
                                                           {{.5f, .5f, 0.f}, {0.f, 1.f, 0.f}},
                                                           {{-.5f, .5f, 0.f}, {0.f, 0.f, 1.f}}};

                if (!demo.LoadGeometry(device.logical_device, device.GetGraphicsQueue(), device.mem_properties,
                                            pipeline.cmd_pool, vertices, indices))
                {
                    std::cerr << "Failed to create triangle!" << std::endl;
                    Delete();
                    return false;
                }

                std::cout << "Successfully setup Vulkan Context. Running Demo!" << std::endl;
                return true;
            }

            bool Delete()
            {
                demo.Clear(device.logical_device);
                pipeline.Clear(device.logical_device);
                device.Clear();
                surface.Clear(instance);

                bool success = VkDebug::Messenger::Delete(instance);
                vkDestroyInstance(instance, VkDebug::Allocation());
                return success;
            }

            bool RenderTriangle()
            {
                // Fake time & Transform Matrices
                static float fake_time = 0.0f;
                fake_time += 0.05f;
                demo.Update(device.logical_device, fake_time);

                // Check for surface capability changes  
                uint8_t changes{};
                if (!surface.UpdateAndGetCapabilityChanges(device.physical_device, changes,
                                                           pipeline.swapchain.extent,
                                                           pipeline.swapchain.surface_transform))
                    return false;

                // If capabilities changed, recreate swapchain
                if (changes != Surface::CapabilityChanges::None)
                    if (!pipeline.OnSurfaceCapabilitiesChanged(device.logical_device, changes, demo.shading,
                                                               surface, device.graphics_family, device.present_family))
                        return false;

                uint32_t next_swapchain_image{};
                if (!pipeline.PrepareRender(device.logical_device, next_swapchain_image))
                {
                    std::cout << "Failed to Prepare Render!" << std::endl;
                    return false;
                }

                if (!demo.UpdateUniforms(device.logical_device, fake_time))
                {
                    std::cerr << "Failed to Update Uniforms!" << std::endl;
                    return false;
                }

                // Bind [Descriptor Sets, Vertex Buffers, Index Buffer] & Draw
                demo.Draw(pipeline.cmd_buffer, pipeline.layout);

                if (!pipeline.SubmitRender(device.GetGraphicsQueue(), next_swapchain_image))
                {
                    std::cerr << "Failed to Submit Render!" << std::endl;
                    return false;
                }
                return true;
            }

          private:

            bool CreateInstance(SDL_Window* window)
            {
                std::cout << "Creating Vulkan Instance." << std::endl;

                // VkInstance - Extensions
                std::vector<const char*> instance_extensions{};
                if (!VkSDLWindow::GetRequiredSDLExtensions(window, instance_extensions))
                    return false;

                std::vector<const char*> layers{};
                LayerProperties::Get(layers);

                VkApplicationInfo appInfo{};
                appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
                appInfo.pApplicationName = "RedEye Engine";
                appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
                appInfo.pEngineName = "RedEye Engine";
                appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
                appInfo.apiVersion = VK_API_VERSION_1_0;

                VkInstanceCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
                createInfo.pApplicationInfo = &appInfo;
                createInfo.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());
                createInfo.ppEnabledExtensionNames = instance_extensions.data();
                createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
                createInfo.ppEnabledLayerNames = layers.data();

                if (vkCreateInstance(&createInfo, VkDebug::Allocation(), &instance) != VK_SUCCESS)
                {
                    std::cerr << "Failed to create Vulkan Instance." << std::endl;
                    return false;
                }

                // VkDebug::Messenger
                if (!LayerProperties::Contains(layers, "VK_LAYER_KHRONOS_validation"))
                {
                    std::cout << "Missing VK_LAYER_KHRONOS_validation layer -> VkDebug::Messenger disabled."
                              << std::endl;
                }
                else if (!VkDebug::Messenger::Create(instance))
                {
                    std::cerr << "Failed to create VkDebug::Messenger." << std::endl;
                    Delete();
                    return false;
                }

                return true;
            }
        };
    } // namespace Vulkan
} // namespace RE
