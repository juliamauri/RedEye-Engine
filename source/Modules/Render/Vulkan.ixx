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

export module Vulkan;

import VkDebug;
import VkSDLWindow;
import Device;
import Shading;
import GraphicPipeline;

namespace Layers
{
    std::vector<VkLayerProperties> available{};

    bool RetrieveAvailable()
    {
        std::cout << "Retrieving available Vulkan layer properties." << std::endl;

        uint32_t layerCount;
        if (vkEnumerateInstanceLayerProperties(&layerCount, nullptr) != VK_SUCCESS)
        {
            std::cerr << "Failed to get Vulkan layer count." << std::endl;
            return false;
        }

        available.resize(layerCount);
        if (vkEnumerateInstanceLayerProperties(&layerCount, available.data()) != VK_SUCCESS)
        {
            std::cerr << "Failed to get Vulkan layer properties." << std::endl;
            return false;
        }

        std::cout << "Retrieved " << available.size() << " Vulkan layers:" << std::endl;
        for (auto& layer : available)
            std::cout << "\t-" << layer.layerName << std::endl;

        return true;
    }

    const std::vector<const char*> prefered = {
        "VK_LAYER_KHRONOS_validation", // requieres vulkan-validationlayers vcpkg
        "VK_LAYER_NV_optimus"          // Ensures discrete NVIDIA GPU usage instead of default integrated GPU
                                       // (Improves Nvidia performance on laptops)
    };

    void Get(std::vector<const char*>& layers)
    {
        for (const auto& prefered_layer : prefered)
        {
            for (const auto& layer : available)
            {
                if (strcmp(layer.layerName, prefered_layer) != 0)
                    continue;

                layers.push_back(layer.layerName);
                break;
            }
        }
    }

    bool Contains(const std::vector<const char*>& layers, const char* layer_to_find)
    {
        for (const auto& layer : layers)
            if (strcmp(layer, layer_to_find) == 0)
                return true;
        return false;
    }
} // namespace Layers


struct ShadingContext
{
  public:
    enum class Type : char
    {
        Fast,
        Instanced
    };

  private:
    Type type = Type::Fast;

    // Fast Shading
    std::vector<FastShading> fast_drawables{};
    FastShading::TransfomMatrices transform_matrices{};

    // Instanced Shading
    std::vector<InstancedShading> instanced_drawables{};
    InstancedShading::ProjectionView proj_view{};

  public:
    void Init(Type _type)
    {
        type = _type;

        // Fast
        fast_drawables.push_back({});
        transform_matrices = {{1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f},
                              {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f},
                              {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f}};

        // Instanced
        instanced_drawables.push_back({});
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

    bool OnCreate(VkDevice logical_device, VkPhysicalDeviceMemoryProperties mem_properties)
    {
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

                for (auto& drawable : instanced_drawables)
                {
                    int i = 0;
                    for (auto& transform : drawable.transforms)
                    {
                        float time = global_time * (1.f + 0.1f * (++i));
                        transform.model[12] = sin(time) * 0.5f; // X-axis
                        transform.model[13] = cos(time) * 0.5f; // Y-axis
                    }

                    if (!drawable.UpdateTransforms(logical_device))
                    {
                        std::cerr << "Failed to Update Transforms Buffer for Instanced Shading!" << std::endl;
                        return false;
                    }
                }
                break;
            default: break;
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
            default: break;
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

    void GetUniformCreateInfos(std::vector<Shading::Uniform::CreateInfo>& out)
    {
        switch (type)
        {
            case Type::Fast:
                out.push_back({&transform_matrices, 0, sizeof(FastShading::TransfomMatrices)});
                break;
            case Type::Instanced:
                out.push_back({&proj_view, 0, sizeof(InstancedShading::ProjectionView)});
                break;
            default:
                break;
        }
    }
};


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

            return Layers::RetrieveAvailable();
        }

        struct Context
        {
            VkInstance instance = VK_NULL_HANDLE;
            VkSurfaceKHR surface = VK_NULL_HANDLE;

            LogicalDevice device{};
            GraphicPipeline pipeline{};

            // State
            VkSurfaceCapabilitiesKHR surface_capabilities{};
            VkExtent2D window_size{};

            // Shading
            ShadingContext shading_context{};

            bool Create(SDL_Window* window, int w, int h)
            {
                std::cout << "Creating Vulkan Context..." << std::endl;
                window_size = {static_cast<uint32_t>(w), static_cast<uint32_t>(h)};
                if (!CreateInstance(window) ||
                    !CreateSurface(window) ||
                    !device.Create(instance, surface) ||
                    !GetSurfaceCapabilities())
                {
                    std::cerr << "Failed to setup environment for Vulkan rendering." << std::endl;
                    Delete();
                    return false;
                }

                std::cout << "Successfully setup Vulkan Instance, Surface & Device." << std::endl
                          << "Creating Graphic Pipeline..." << std::endl;
                shading_context.Init(ShadingContext::Type::Fast);
                if (!pipeline.Create(shading_context.GerFirst(), device, surface, surface_capabilities, window_size))
                {
                    std::cerr << "Failed to setup environment for Vulkan rendering." << std::endl;
                    Delete();
                    return false;
                }

                std::cout << "Graphic Pipeline created." << std::endl 
                          << "Setting up Demo..." << std::endl;
                if (!CreateTriangle())
                {
                    std::cerr << "Failed to setup Demo." << std::endl;
                    Delete();
                    return false;
                }

                std::cout << "Successfully setup Vulkan Context. Running Demo!" << std::endl;
                return true;
            }

            bool Delete()
            {
                shading_context.Clear(device.logical_device);

                pipeline.Delete();

                if (device.logical_device != VK_NULL_HANDLE)
                    vkDestroyDevice(device.logical_device, VkDebug::Allocation());
                if (surface != VK_NULL_HANDLE)
                    vkDestroySurfaceKHR(instance, surface, VkDebug::Allocation());

                if (!VkDebug::Messenger::Delete(instance))
                    return false;

                if (instance != VK_NULL_HANDLE)
                    vkDestroyInstance(instance, VkDebug::Allocation());

                return true;
            }

            bool RenderTriangle()
            {
                // Fake time & Transform Matrices
                static float fake_time = 0.0f;
                fake_time += 0.05f;
                shading_context.Update(device.logical_device, fake_time);

                uint32_t next_swapchain_image{};
                if (!GetSurfaceCapabilities() ||
                    !pipeline.PrepareRender(shading_context.GerFirst(), device, surface, surface_capabilities,
                                            window_size, next_swapchain_image))
                {
                    std::cerr << "Failed to Prepare Render!" << std::endl;
                    return false;
                }

                if (!shading_context.UpdateUniforms(device.logical_device, fake_time))
                {
                    std::cerr << "Failed to Update Uniforms!" << std::endl;
                    return false;
                }

                // Bind [Descriptor Sets, Vertex Buffers, Index Buffer] & Draw
                shading_context.Draw(pipeline.commandBuffer, pipeline.pipelineLayout);

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
                Layers::Get(layers);

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
                if (!Layers::Contains(layers, "VK_LAYER_KHRONOS_validation"))
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

            bool CreateSurface(SDL_Window* window)
            {
                if (SDL_Vulkan_CreateSurface(window, instance, &surface) == SDL_TRUE)
                    return true;

                std::cerr << "Failed to get surface capabilities!" << std::endl;
                return false;
            }

            bool GetSurfaceCapabilities()
            {
                if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physical_device, surface, &surface_capabilities) ==
                    VK_SUCCESS)
                    return true;

                std::cerr << "Failed to get surface capabilities!" << std::endl;
                return false;
            }

            bool CreateTriangle()
            {
                std::vector<uint32_t> indices = {0, 1, 2};
                std::vector<Shading::PosColor> vertices = {{{ .0f, -.5f, 0.f}, {1.f, 0.f, 0.f}},
                                                           {{ .5f,  .5f, 0.f}, {0.f, 1.f, 0.f}},
                                                           {{-.5f, .5f, 0.f}, {0.f, 0.f, 1.f}}};
                std::vector<Shading::Uniform::CreateInfo> uniform_create_infos{};
                shading_context.GetUniformCreateInfos(uniform_create_infos);

                if (!shading_context.GerFirst()->Create(
                    device.logical_device, device.GetGraphicsQueue(), device.mem_properties,
                    pipeline.commandPool, pipeline.descriptorPool, pipeline.descriptorSetLayout,
                    vertices, indices, uniform_create_infos) ||
                    !shading_context.OnCreate(device.logical_device, device.mem_properties))
                {
                    std::cerr << "Failed to create triangle!" << std::endl;
                    return false;
                }

                return true;
            }
        };
    } // namespace Vulkan
} // namespace RE
