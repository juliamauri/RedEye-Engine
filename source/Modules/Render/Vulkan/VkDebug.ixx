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

export module VkDebug;

VkAllocationCallbacks* allocation_callbacks = nullptr;
VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

export namespace VkDebug
{
    const VkAllocationCallbacks* Allocation()
    {
        return allocation_callbacks;
    }

    void SetAllocation(VkAllocationCallbacks* callbacks)
    {
        allocation_callbacks = callbacks;
    }

    namespace Messenger
    {
        bool Create(VkInstance instance)
        {
            // Retrieve Create Function
            const char* func_name = "vkCreateDebugUtilsMessengerEXT";
            auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, func_name);
            if (func == nullptr)
            {
                std::cerr << "Failed to retrieve " << func_name << "!" << std::endl;
                return false;
            }

            // Call Create Function
            VkDebugUtilsMessengerCreateInfoEXT create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            create_info.pfnUserCallback = DebugCallback;

            if (func(instance, &create_info, allocation_callbacks, &messenger) != VK_SUCCESS)
            {
                std::cerr << "Failed to Create debug messenger!" << std::endl;
                return false;
            }

            return true;
        }

        bool Delete(VkInstance instance)
        {
            if (messenger == VK_NULL_HANDLE)
                return true;

            // Retrieve Destroy Function
            const char* func_name = "vkDestroyDebugUtilsMessengerEXT";
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, func_name);
            if (func == nullptr)
            {
                std::cerr << "Failed to retrieve " << func_name << "!" << std::endl;
                return false;
            }

            func(instance, messenger, allocation_callbacks);
            return true;
        }
    }; // namespace Messenger
} // namespace VulkanDebug