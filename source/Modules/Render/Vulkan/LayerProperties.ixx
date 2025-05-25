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
#include <vector>

export module LayerProperties;

export namespace LayerProperties
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

    void Get(std::vector<const char*>& layers,
             const std::vector<const char*> prefered = {
                 "VK_LAYER_KHRONOS_validation", // requieres vulkan-validationlayers vcpkg
                 "VK_LAYER_NV_optimus"          // Ensures discrete NVIDIA GPU usage instead of default integrated GPU
                                                // (Improves Nvidia performance on laptops)
             })
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
} // namespace LayerProperties
