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

export module Utility;

export namespace Delete
{
    inline void Ptr(auto*& ptr)
    {
        delete ptr;
        ptr = nullptr;
    }

    inline void Array(auto*& array_ptr)
    {
        delete[] array_ptr;
        array_ptr = nullptr;
    }
}

export template <typename Collection, typename MemberFunc>
inline void CLEAR(Collection& collection, MemberFunc clear_call)
{
    for (auto& item : collection) (item.*clear_call)();
    collection.clear();
}

export template <typename Collection, typename MemberFunc, typename... Args>
inline void CLEAR(Collection& collection, MemberFunc clear_call, Args&&... args)
{
    for (auto& item : collection) (item.*clear_call)(args...);
    collection.clear();
}
