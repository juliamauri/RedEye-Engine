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

#include <fstream>
#include <iostream>
#include <memory>

export module Files;

export struct File
{
    const char* file_path = nullptr;
    std::unique_ptr<char[]> buffer = nullptr;
    std::streamsize size = 0;

    bool Read()
    {
        if (file_path == nullptr)
        {
            std::cerr << "Failed to open file: file_path is nullptr" << std::endl;
            return false;
        }

        std::ifstream file(file_path, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file: " << file_path << std::endl;
            return false;
        }

        size = file.tellg();
        file.seekg(0, std::ios::beg);

        buffer = std::make_unique<char[]>(size);
        if (!file.read(buffer.get(), size))
        {
            std::cerr << "Failed to read file: " << file_path << std::endl;
            return false;
        }

        file.close();
        return true;
    }

    void Clear()
    {
        buffer = nullptr;
        size = 0;
    }
};
