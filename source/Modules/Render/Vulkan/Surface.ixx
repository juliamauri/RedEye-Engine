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

#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <utility>

export module Surface;

import VkDebug;

export struct Surface
{
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSurfaceCapabilitiesKHR capabilities{};
    VkExtent2D window_size{};
    VkSurfaceFormatKHR format{};
    VkPresentModeKHR present_mode{};

    bool Create(SDL_Window* window, VkInstance instance, VkExtent2D _window_size,
                VkSurfaceFormatKHR _format = {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                VkPresentModeKHR _present_mode = VK_PRESENT_MODE_FIFO_KHR)
    {
        std::cout << "Creating SDL Vulkan Surface." << std::endl;

        if (SDL_Vulkan_CreateSurface(window, instance, &surface) != SDL_TRUE)
        {
            std::cerr << "Failed to Create SDL Vulkan Surface." << std::endl;
            return false;
        }

        window_size = _window_size;
        format = _format;
        present_mode = _present_mode;
        return true;
    }

    void Clear(VkInstance instance)
    {
        vkDestroySurfaceKHR(instance, surface, VkDebug::Allocation());
        surface = VK_NULL_HANDLE;
        window_size = {};
        format = {};
        present_mode = {};
    }

    enum CapabilityChanges : uint8_t
    {
        None = 0,
        Transform = 1 << 0,
        Extent = 1 << 1
    };

    bool UpdateCapabilities(VkPhysicalDevice physical_device)
    {
        // std::cout << "Updating SDL Vulkan Surface Capabilities." << std::endl;

        if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities) != VK_SUCCESS)
        {
            std::cerr << "Failed to get Vulkan Physical Device Surface Capabilities." << std::endl;
            return false;
        }

        return true;
    }

    bool UpdateAndGetCapabilityChanges(VkPhysicalDevice physical_device, 
                            uint8_t& out_changes,
                            const VkExtent2D& prev_extent, 
                            const VkSurfaceTransformFlagBitsKHR& prev_surface_transform)
    {
        if (!UpdateCapabilities(physical_device))
            return false;

        out_changes = 0;
        if (capabilities.currentTransform != prev_surface_transform)
        {
            std::cout << "Surface transformation changed. ";
            out_changes |= CapabilityChanges::Transform;
        }
        if (capabilities.currentExtent.width != prev_extent.width ||
            capabilities.currentExtent.height != prev_extent.height)
        {
            std::cout << "Surface extent changed. ";
            out_changes |= CapabilityChanges::Extent;
        }

        return true;
    }

    VkExtent2D GetExtent() const
    {
        // Fixed size surface
        if (capabilities.currentExtent.width != UINT32_MAX)
            return capabilities.currentExtent;

        // Resizable surface
        return {
            std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, window_size.width)),
            std::max(capabilities.minImageExtent.height,
                     std::min(capabilities.maxImageExtent.height, window_size.height))};
    }

    uint32_t MinImageCount() const
    {
        return capabilities.maxImageCount > 0 && capabilities.minImageCount + 1 > capabilities.maxImageCount
                   ? capabilities.maxImageCount
                   : capabilities.minImageCount + 1;
    }
};
