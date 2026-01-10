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

#include <chrono>
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
import GraphicPipeline;
import ShadingContext;

import ECS;

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

            ECS ecs{};
            ShadingContext::Manager demo{};

            bool Create(SDL_Window* window, int w, int h)
            {
                if (!CreateInstance(window))
                    return false;

                if (!surface.Create(window, instance, {static_cast<uint32_t>(w), static_cast<uint32_t>(h)}) ||
                    !device.Create(instance, surface) || 
                    !surface.UpdateCapabilities(device.physical_device) ||
                    !demo.Init(device.logical_device, device.mem_properties, ShadingContext::Type::Mesh) ||
                    !pipeline.Create(device.logical_device, demo.ShadingSpecs(), surface,
                                     device.graphics_family, device.present_family))
                {
                    Delete();
                    return false;
                }

                std::cout << "Graphic Pipeline created." << std::endl;

                {
                    Mesh triangle{};
                    triangle.MakeTriangle(0.5f, {0.5f, 0.0f, 0.5f});

                    Material triangle_material{};
                    triangle_material.SetColorLayer(Material::ColorLayer::diffuse, {1.f, 0.f, 0.f});

                    std::cout << "Creating right red triangle for Demo..." << std::endl;
                    if (!demo.LoadMesh(device.logical_device, device.GetGraphicsQueue(), device.mem_properties,
                                       pipeline.cmd_pool, triangle, triangle_material))
                    {
                        std::cerr << "Failed to create triangle!" << std::endl;
                        Delete();
                        return false;
                    }
                }
                {
                    Mesh cube{};
                    cube.MakeCube(0.5f, {-0.5f, 0.0f, 0.5f});

                    Material cube_material{};
                    cube_material.SetColorLayer(Material::ColorLayer::diffuse, {0.f, 1.f, 0.f});

                    std::cout << "Creating left green cube for Demo..." << std::endl;
                    if (!demo.LoadMesh(device.logical_device, device.GetGraphicsQueue(), device.mem_properties,
                                       pipeline.cmd_pool, cube, cube_material))
                    {
                        std::cerr << "Failed to create cube!" << std::endl;
                        Delete();
                        return false;
                    }
                }

                ecs.AddCamera().SetOrthographic({0.f, 0.f, -4.0f});
                //std::cout << "Importing sponza.obj" << std::endl;
                //if (!ecs.Import("C:/Users/Cumus/Desktop/Sponza-master/sponza.obj"))
                //{
                //    std::cerr << "Failed to load model!" << std::endl;
                //    Delete();
                //    return false;
                //}
                //
                //for (const auto& mesh : ecs.loaded_meshes)
                //{
                //    if (!demo.LoadMesh(device.logical_device, device.GetGraphicsQueue(), device.mem_properties,
                //                           pipeline.cmd_pool, mesh, ecs.loaded_materials[mesh.material_index]))
                //    {
                //        std::cerr << "Failed to upload model: " << mesh.name << std::endl;
                //        Delete();
                //        return false;
                //    }
                //}

                std::cout << "Successfully setup Vulkan Context. Running Demo!" << std::endl;
                return true;
            }

            bool Delete()
            {
                ecs.Clear();
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
                auto ms_start = std::chrono::high_resolution_clock::now();

                if (!demo.PreparePool(device.logical_device))
                {
                    std::cerr << "Failed to create descriptor sets or pool on preparing to render!" << std::endl;
                    return false;
                }

                // Fake time & Transform Matrices
                static float fake_time = 0.0f;
                fake_time += 0.05f;
                if (!demo.Update(device.logical_device, fake_time, ecs))
                {
                    std::cerr << "Failed to Update Demo!" << std::endl;
                    return false;
                }

                // Check for surface capability changes  
                uint8_t changes{};
                if (!surface.UpdateAndGetCapabilityChanges(device.physical_device, changes,
                                                           pipeline.swapchain.extent,
                                                           pipeline.swapchain.surface_transform))
                    return false;

                // If capabilities changed, recreate swapchain
                if (changes != Surface::CapabilityChanges::None)
                {
                    if (!pipeline.OnSurfaceCapabilitiesChanged(device.logical_device, changes, demo.ShadingSpecs(),
                                                               surface, device.graphics_family, device.present_family))
                        return false;

                    if (changes & Surface::CapabilityChanges::Extent)
                    {
                        VkExtent2D extent = surface.GetExtent();
                        ecs.MainCamera().SetBounds(extent.width, extent.height);
                    }
                }

                uint32_t next_swapchain_image{};
                if (!pipeline.PrepareRender(device.logical_device, next_swapchain_image))
                {
                    std::cerr << "Failed to Prepare Render!" << std::endl;
                    return false;
                }

                // Bind [Descriptor Sets, Vertex Buffers, Index Buffer] & Draw
                demo.Draw(pipeline.cmd_buffer, pipeline.layout);

                if (!pipeline.SubmitRender(device.GetGraphicsQueue(), next_swapchain_image))
                {
                    std::cerr << "Failed to Submit Render!" << std::endl;
                    return false;
                }

                //std::cout << "Render took: "
                //          << std::chrono::duration_cast<std::chrono::milliseconds>(
                //                 std::chrono::high_resolution_clock::now() - ms_start)
                //                 .count()
                //          << " ms" << std::endl;

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
