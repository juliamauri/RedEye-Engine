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

#include <iostream>
#include <vector>

export module Swapchain;

import VkDebug;
import Surface;

export struct Swapchain
{
    VkSwapchainKHR id = VK_NULL_HANDLE;

    VkExtent2D extent{};
    VkSurfaceTransformFlagBitsKHR surface_transform;

    std::vector<VkImage> images{};
    std::vector<VkImageView> image_views{};    // Image views for swapchain images.
    std::vector<VkFramebuffer> framebuffers{}; // Framebuffers for swapchain images.

    bool Create(VkDevice logical_device, const Surface& surface, uint32_t graphics_family, uint32_t present_family)
    {
        std::cout << "Creating Swapchain..." << std::endl;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface.surface;
        createInfo.minImageCount = surface.MinImageCount();
        createInfo.imageFormat = surface.format.format;
        createInfo.imageColorSpace = surface.format.colorSpace;
        createInfo.imageExtent = extent = surface.GetExtent();
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queueFamilyIndices[] = {graphics_family, present_family};
        if (graphics_family != present_family)
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

        createInfo.preTransform = surface_transform = surface.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = surface.present_mode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = id;

        if (vkCreateSwapchainKHR(logical_device, &createInfo, VkDebug::Allocation(), &id) != VK_SUCCESS)
        {
            std::cerr << "Failed to create Swapchain." << std::endl;
            return false;
        }

        return true;
    }
    void Clear(VkDevice logical_device)
    {
        ClearResources(logical_device);
        vkDestroySwapchainKHR(logical_device, id, VkDebug::Allocation());
    }

    bool Recreate(VkDevice logical_device, const Surface& surface, uint32_t graphics_family, uint32_t present_family,
                  VkRenderPass render_pass)
    {
        ClearResources(logical_device);
        return Create(logical_device, surface, graphics_family, present_family) &&
               RetrieveImages(logical_device, surface.format, render_pass);
    }

    bool RetrieveImages(VkDevice logical_device, const VkSurfaceFormatKHR& surface_format, VkRenderPass render_pass)
    {
        // Get VkImages
        uint32_t image_count;
        if (vkGetSwapchainImagesKHR(logical_device, id, &image_count, nullptr) != VK_SUCCESS)
        {
            std::cerr << "Failed to get swapchain image count!" << std::endl;
            return false;
        }

        images.resize(image_count);
        if (vkGetSwapchainImagesKHR(logical_device, id, &image_count, images.data()) != VK_SUCCESS)
        {
            std::cerr << "Failed to get swapchain images!" << std::endl;
            return false;
        }

        // Get VkImageViews & VkFramebuffers
        image_views.resize(image_count);
        framebuffers.resize(image_count);
        for (auto i = 0; i < image_count; i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = surface_format.format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(logical_device, &createInfo, VkDebug::Allocation(), &image_views[i]) != VK_SUCCESS)
            {
                std::cerr << "Failed to create image view " << i + 1 << "/" << image_count << "!" << std::endl;
                return false;
            }

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = render_pass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = &image_views[i];
            framebufferInfo.width = extent.width;
            framebufferInfo.height = extent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(logical_device, &framebufferInfo, VkDebug::Allocation(), &framebuffers[i]) !=
                VK_SUCCESS)
            {
                std::cerr << "Failed to create framebuffer " << i + 1 << " / " << image_count << "!" << std::endl;
                return false;
            }
        }

        return true;
    }

    bool GetNextImage(VkDevice logical_device, uint32_t& next_swapchain_image, VkSemaphore imageAvailableSemaphore,
                      VkFence fence = VK_NULL_HANDLE, uint64_t timeout = UINT64_MAX) const
    {
        if (vkAcquireNextImageKHR(logical_device, id, timeout, imageAvailableSemaphore, fence, &next_swapchain_image) !=
            VK_SUCCESS)
        {
            std::cerr << "Failed to acquire next swapchain image." << std::endl;
            return false;
        }
        return true;
    }

  private:
    void ClearResources(VkDevice logical_device)
    {
        for (auto framebuffer : framebuffers)
            vkDestroyFramebuffer(logical_device, framebuffer, VkDebug::Allocation());
        for (auto imageView : image_views)
            vkDestroyImageView(logical_device, imageView, VkDebug::Allocation());
    }
};
