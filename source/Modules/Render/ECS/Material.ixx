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

#include <string>
#include <array>
#include <vector>
#include <map>

#include <assimp/material.h>

export module Material;

export struct Material
{
    enum class Flags : short
    {
        twosided = 1 << 0,
        blend_func = 1 << 1
    };

    enum class ColorLayer : short
    {
        diffuse,
        specular,
        ambient,
        emissive,
        transparent,
        MAX
    };

    struct Uniform
    {
        float diffuse[3] = {0.f, 0.f, 0.f};
    };

    std::map<aiTextureType, std::vector<std::string>> textures{};
    aiShadingMode shading = aiShadingMode_Flat;
    unsigned short flags = 0;
    std::array<aiColor3D, static_cast<short>(ColorLayer::MAX)> single_colors{};
    float opacity = 1.f;
    float shininess = 1.f;
    float shininess_strength = 1.f;
    float refraccti = 1.f;
    std::string name{};

    void Parse(const aiMaterial& ai_material)
    {
        textures.clear();
        for (auto texture_type : {
            aiTextureType_DIFFUSE,
            aiTextureType_SPECULAR, 
            aiTextureType_AMBIENT, 
            aiTextureType_EMISSIVE,
            aiTextureType_OPACITY,
            aiTextureType_SHININESS,

            aiTextureType_HEIGHT,
            aiTextureType_NORMALS, 
            aiTextureType_REFLECTION,
            aiTextureType_UNKNOWN})
        {
            auto texture_count = ai_material.GetTextureCount(texture_type);
            if (texture_count == 0)
                continue;

            aiString path{};
            for (unsigned int i = 0; i < texture_count; i++)
                if (ai_material.GetTexture(texture_type, i, &path) == AI_SUCCESS)
                    textures[texture_type].push_back(path.C_Str());
        }

        int tmp = 0;
        shading = ai_material.Get(AI_MATKEY_SHADING_MODEL, tmp) == AI_SUCCESS ?
             static_cast<aiShadingMode>(tmp) : aiShadingMode_Flat;

        flags = 0;
        if (ai_material.Get(AI_MATKEY_TWOSIDED, tmp) == AI_SUCCESS && tmp)
            flags |= static_cast<short>(Flags::twosided);
        if (ai_material.Get(AI_MATKEY_BLEND_FUNC, tmp) == AI_SUCCESS && tmp)
            flags |= static_cast<short>(Flags::blend_func);

        ai_material.Get(AI_MATKEY_COLOR_DIFFUSE, single_colors[static_cast<short>(ColorLayer::diffuse)]);
        ai_material.Get(AI_MATKEY_COLOR_SPECULAR, single_colors[static_cast<short>(ColorLayer::specular)]);
        ai_material.Get(AI_MATKEY_COLOR_AMBIENT, single_colors[static_cast<short>(ColorLayer::ambient)]);
        ai_material.Get(AI_MATKEY_COLOR_EMISSIVE, single_colors[static_cast<short>(ColorLayer::emissive)]);
        ai_material.Get(AI_MATKEY_COLOR_TRANSPARENT, single_colors[static_cast<short>(ColorLayer::transparent)]);
        
        ai_material.Get(AI_MATKEY_OPACITY, opacity);
        ai_material.Get(AI_MATKEY_SHININESS, shininess);
        ai_material.Get(AI_MATKEY_SHININESS_STRENGTH, shininess_strength);
        ai_material.Get(AI_MATKEY_REFRACTI, refraccti);

        { // Context bound for aiString is 1028 bytes
            aiString ai_name{};
            name = ai_material.Get(AI_MATKEY_NAME, ai_name) == AI_SUCCESS ? ai_name.C_Str() : "undefined";
        }
    }

    inline bool IsOpaque() const { return opacity >= 1.f; }
    inline bool IsTransparent() const { return opacity < 1.f; }
    inline bool IsTwoSided() const { return (flags & static_cast<short>(Flags::twosided)) != 0; }
    inline bool HasBlend() const { return (flags & static_cast<short>(Flags::blend_func)) != 0; }

    void SetColorLayer(Material::ColorLayer layer, aiColor3D color)
    {
        single_colors[static_cast<short>(layer)] = color;
    }

    Uniform ToUniform() const
    {
        switch (shading)
        {
            default:
            {
                auto& diffuse_color = single_colors[static_cast<short>(Material::ColorLayer::diffuse)];
                return {diffuse_color.r, diffuse_color.g, diffuse_color.b};
            }
        }
    }
};
