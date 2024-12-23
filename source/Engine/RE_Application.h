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

#pragma once

#include <cstdint>

namespace Application
{
    bool Init(int argc, char* argv[]);
    void MainLoop();
    void CleanUp();

    namespace
    {
        int argc = 0;
        char** argv = nullptr;
        uint8_t flags = 0;
    } // namespace

    enum class Flag : uint8_t
    {
        LOAD_CONFIG = 1 << 0,
        SAVE_CONFIG = 1 << 1,
        WANT_TO_QUIT = 1 << 2,
        SAVE_ON_EXIT = 1 << 3
    };

    inline void AddFlag(Flag flag)
    {
        flags |= static_cast<uint8_t>(flag);
    }

    inline void RemoveFlag(Flag flag)
    {
        flags &= ~static_cast<uint8_t>(flag);
    }

    inline bool HasFlag(Flag flag)
    {
        return flags & static_cast<uint8_t>(flag);
    }

    inline void Quit()
    {
        AddFlag(Flag::WANT_TO_QUIT);
    }

}; // namespace Application
