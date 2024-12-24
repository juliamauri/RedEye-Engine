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
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

export module Vulkan;

export namespace RE
{
    struct VulkanContext
    {
        VkInstance instance;
        VkDevice device;
        VkSurfaceKHR surface;

        bool Create(SDL_Window* window)
        {
            return CreateInstance(window) && CreateSurface(window) && FindDevice();
        }

        bool GetExtensions(SDL_Window* window, std::vector<const char*>& out_extensions)
        {
            uint32_t extensionCount = 0;
            if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr))
            {
                std::cerr << "Failed to get Vulkan instance extension count." << std::endl;
                return false;
            }

            out_extensions.resize(extensionCount);
            if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, out_extensions.data()))
            {
                std::cerr << "Failed to get Vulkan instance extensions." << std::endl;
                return false;
            }

            return true;
        }

        bool CreateInstance(SDL_Window* window)
        {
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

            std::cout << "Retrieving Vulkan instance extensions." << std::endl;
            std::vector<const char*> extensions;
            if (GetExtensions(window, extensions))
            {
                std::cout << "Retrieved " << extensions.size() << "/" << extensions.capacity()
                          << " Vulkan instance extensions:" << std::endl;
                for (const char* extension : extensions)
                    std::cout << "\t-" << extension << std::endl;
            }
            else
                return false;

            createInfo.enabledExtensionCount = extensions.size();
            createInfo.ppEnabledExtensionNames = extensions.data();

            if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
            {
                std::cerr << "Failed to create Vulkan instance." << std::endl;
                return false;
            }
            std::cout << "Created Vulkan instance." << std::endl;
            return true;
        }

        bool CreateSurface(SDL_Window* window)
        {
            if (!SDL_Vulkan_CreateSurface(window, instance, &surface))
            {
                std::cerr << "Failed to create Vulkan surface." << std::endl;
                vkDestroyInstance(instance, nullptr);
                return false;
            }
            std::cout << "Created Vulkan SDL surface." << std::endl;
            return true;
        }

        bool FindDevice()
        {
            // Select physical & logic device
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
            if (deviceCount == 0)
            {
                std::cerr << "Failed to find GPUs with Vulkan support." << std::endl;
                vkDestroySurfaceKHR(instance, surface, nullptr);
                vkDestroyInstance(instance, nullptr);
                return false;
            }
            std::cout << "Found " << deviceCount << " GPUs with Vulkan support." << std::endl;

            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

            VkPhysicalDevice physicalDevice = devices[0];

            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = 0;
            queueCreateInfo.queueCount = 1;
            float queuePriority = 1.0f;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            VkDeviceCreateInfo deviceCreateInfo{};
            deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            deviceCreateInfo.queueCreateInfoCount = 1;
            deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

            if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS)
            {
                std::cerr << "Failed to create logical device." << std::endl;
                vkDestroySurfaceKHR(instance, surface, nullptr);
                vkDestroyInstance(instance, nullptr);
                return false;
            }
            std::cout << "Created Vulkan logical device." << std::endl;

            return true;
        }

        void Delete()
        {
            vkDestroySurfaceKHR(instance, surface, nullptr);
            vkDestroyDevice(device, nullptr);
            vkDestroyInstance(instance, nullptr);
        }

        void RenderTriangle(SDL_Window* window)
        {

        }
    };
} // namespace RE