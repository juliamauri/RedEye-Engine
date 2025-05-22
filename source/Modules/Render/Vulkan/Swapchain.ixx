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

#include <utility>

export module Swapchain;

import VkDebug;
import Device;

VkExtent2D GetExtent(const VkSurfaceCapabilitiesKHR& capabilities, const VkExtent2D& window_size)
{
    // Fixed size surface
    if (capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;

    // Resizable surface
    return {
        std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, window_size.width)),
        std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, window_size.height))};
}

uint32_t GetMinImageCount(const VkSurfaceCapabilitiesKHR& capabilities)
{
    return capabilities.maxImageCount > 0 && capabilities.minImageCount + 1 > capabilities.maxImageCount
               ? capabilities.maxImageCount
               : capabilities.minImageCount + 1;
}

export struct Swapchain
{
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;

    // State
    VkExtent2D extent;
    VkSurfaceTransformFlagBitsKHR surface_transform;

    bool Create(const LogicalDevice& device, VkSurfaceKHR surface, const VkSurfaceCapabilitiesKHR& capabilities,
                const VkExtent2D& window_size, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE)
    {
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = GetMinImageCount(capabilities);
        createInfo.imageFormat = device.surface_format.format;
        createInfo.imageColorSpace = device.surface_format.colorSpace;
        createInfo.imageExtent = extent = GetExtent(capabilities, window_size);
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queueFamilyIndices[] = {device.graphics_family, device.present_family};
        if (device.graphics_family != device.present_family)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = surface_transform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = device.present_mode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = oldSwapchain;
        return vkCreateSwapchainKHR(device.logical_device, &createInfo, VkDebug::Allocation(), &swapchain) ==
               VK_SUCCESS;
    }

    enum CapabilityChanges : uint8_t
    {
        None = 0,
        Transform = 1 << 0,
        Extent = 1 << 1
    };

    uint8_t GetChanges(const VkSurfaceCapabilitiesKHR& new_capabilities)
    {
        uint8_t changes = 0;

        if (new_capabilities.currentTransform != surface_transform)
            changes |= CapabilityChanges::Transform;

        if (new_capabilities.currentExtent.width != extent.width ||
            new_capabilities.currentExtent.height != extent.height)
            changes |= CapabilityChanges::Extent;

        return changes;
    }
};
