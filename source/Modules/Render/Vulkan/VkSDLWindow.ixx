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

#include <iostream>
#include <vector>

export module VkSDLWindow;

export namespace VkSDLWindow
{
    bool GetRequiredSDLExtensions(SDL_Window* window, std::vector<const char*>& extensions)
    {
        std::cout << "Retrieving Vulkan window extensions." << std::endl;

        uint32_t extensionCount = 0;
        if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr))
        {
            std::cerr << "Failed to SDL get Vulkan instance extension count." << std::endl;
            return false;
        }
        if (extensionCount == 0)
        {
            std::cout << "Retrieved 0 Vulkan window extensions." << std::endl;
            return true;
        }

        extensions.resize(extensionCount);
        if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensions.data()))
        {
            std::cerr << "Failed to SDL get Vulkan instance extensions." << std::endl;
            return false;
        }

        std::cout << "Retrieved " << extensions.size() << " Vulkan surface extensions:" << std::endl;
        for (const char* extension : extensions)
            std::cout << "\t-" << extension << std::endl;

        return true;
    }

    void LogSizeProperties(SDL_Window* window)
    {
        int logicalWidth, logicalHeight;
        SDL_GetWindowSize(window, &logicalWidth, &logicalHeight);

        int drawableWidth, drawableHeight;
        SDL_Vulkan_GetDrawableSize(window, &drawableWidth, &drawableHeight);

        float dpiScaleX = static_cast<float>(drawableWidth) / logicalWidth;
        float dpiScaleY = static_cast<float>(drawableHeight) / logicalHeight;

        std::cout << "Window Properties: " << std::endl;
        std::cout << "\t  Logical size: " << logicalWidth << "x" << logicalHeight << "\n";
        std::cout << "\t  Drawable size: " << drawableWidth << "x" << drawableHeight << "\n";
        std::cout << "\t  DPI Scale: " << dpiScaleX << ", " << dpiScaleY << "\n";
    }
}