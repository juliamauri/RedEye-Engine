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

#include <vector>
#include <array>
#include <string>
#include <memory>

#include <assimp/mesh.h>
#include <Geometry/AABB.h>

export module Mesh;

import Material;
import Utility;

export struct Mesh
{
    std::string name{};
    math::AABB aabb{};
    int material_index = -1;
    unsigned int primitive_types = 0;
    unsigned int vertex_count = 0;
    unsigned int face_count = 0;

    // Data Arrays
    std::unique_ptr<float[]> vertices = nullptr;
    std::unique_ptr<unsigned int[]> indices = nullptr;
    std::unique_ptr<float[]> normals = nullptr;
    std::unique_ptr<float[]> tangents = nullptr;
    std::unique_ptr<float[]> bitangents = nullptr;


    // Vertex Colors & UVs 
    // aiColor4D* vertex_colors = nullptr;
    // C_STRUCT aiVector3D* mTextureCoords[AI_MAX_NUMBER_OF_TEXTURECOORDS]; // UV channels (Vertex texture coordinates)
    // unsigned int mNumUVComponents[AI_MAX_NUMBER_OF_TEXTURECOORDS]; // number of components for a given UV channel
    // C_STRUCT aiString** mTextureCoordsNames;                             // Vertex UV stream names

    // Skeleton & Animations
    // unsigned int mNumBones;
    // C_STRUCT aiBone** mBones;
    // unsigned int mNumAnimMeshes;
    // C_STRUCT aiAnimMesh** mAnimMeshes;
    // enum aiMorphingMethod mMethod;

    void Parse(const aiMesh& ai_mesh, const std::vector<unsigned int>& material_indexes)
    {
        name = ai_mesh.mName.C_Str();
        aabb = {
            {
                ai_mesh.mAABB.mMin.x,
                ai_mesh.mAABB.mMin.y,
                ai_mesh.mAABB.mMin.z
            }, 
            {
                ai_mesh.mAABB.mMax.x,
                ai_mesh.mAABB.mMax.y,
                ai_mesh.mAABB.mMax.z
            }};

        material_index = material_indexes[ai_mesh.mMaterialIndex];
        primitive_types = ai_mesh.mPrimitiveTypes;
        size_t v_count = vertex_count = ai_mesh.mNumVertices;
        size_t f_count = face_count = ai_mesh.mNumFaces;

        // Vertices
        if (vertex_count > 0 && ai_mesh.mVertices)
        {
            vertices = std::make_unique<float[]>(v_count * 3);
            memcpy(vertices.get(), ai_mesh.mVertices, v_count * 3 * sizeof(float));
        }
        
        // Indices
        if (face_count > 0 && ai_mesh.mFaces)
        {
            indices = std::make_unique<unsigned int[]>(f_count * 3);
            unsigned int* cursor = indices.get();
            for (unsigned int i = 0; i < face_count; i++)
            {
                memcpy(cursor, ai_mesh.mFaces[i].mIndices, 3 * sizeof(unsigned int));
                cursor += 3;
            }
        }
        
        // Normals
        normals = nullptr;
        if (ai_mesh.HasNormals())
        {
            normals = std::make_unique<float[]>(v_count * 3);
            memcpy(normals.get(), ai_mesh.mNormals, v_count * 3 * sizeof(float));
        }

        // Tangents & Bitangents
        tangents = nullptr;
        bitangents = nullptr;
        if (ai_mesh.HasTangentsAndBitangents())
        {
            tangents = std::make_unique<float[]>(v_count * 3);
            memcpy(tangents.get(), ai_mesh.mTangents, v_count * 3 * sizeof(float));
            bitangents = std::make_unique<float[]>(v_count * 3);
            memcpy(bitangents.get(), ai_mesh.mBitangents, v_count * 3 * sizeof(float));
        }
    }

    void MakeTriangle(float size = 1.f, math ::float3 pos = math::float3::zero)
    {
        std::array<unsigned int, 3> _indices = {0, 1, 2};
        std::array<math::float3, 3> _vertices = {math::float3(0.f, -0.5f, 0.f) * size + pos,
                                                math::float3(0.5f, 0.5f, 0.f) * size + pos,
                                                math::float3(-0.5f, 0.5f, 0.f) * size + pos};
        std::array<math::float3, 3> _normals = {math::float3::unitZ, math::float3::unitZ, math::float3::unitZ};

        auto v_count = _vertices.size();
        auto i_count = _indices.size();

        name = "triangle";
        aabb = AABB::MinimalEnclosingAABB(_vertices.data(), v_count);
        material_index = -1;
        primitive_types = 0;
        vertex_count = v_count;
        face_count = i_count / 3;

        vertices = std::make_unique<float[]>(v_count * 3);
        indices = std::make_unique<unsigned int[]>(i_count);
        normals = std::make_unique<float[]>(v_count * 3);

        memcpy(vertices.get(), _vertices.data(), v_count * 3 * sizeof(float));
        memcpy(indices.get(), _indices.data(), i_count * sizeof(unsigned int));
        memcpy(normals.get(), _normals.data(), v_count * 3 * sizeof(float));
    }

    void MakeCube(float size = 1.f, math ::float3 pos = math::float3::zero)
    {
        std::array<unsigned int, 36> _indices = {

            0, 1, 2, 2, 3, 0, // -Z
            4, 5, 6, 6, 7, 4, // +Z
            0, 3, 7, 7, 4, 0, // -X
            1, 5, 6, 6, 2, 1, // +X
            0, 1, 5, 5, 4, 0, // -Y
            3, 2, 6, 6, 7, 3  // +Y
        };

        float h = size * 0.5f;
        std::array<math::float3, 8> _vertices = {
            pos + math::float3(-h, -h, -h), // 0
            pos + math::float3(h, -h, -h),  // 1
            pos + math::float3(h, h, -h),   // 2
            pos + math::float3(-h, h, -h),  // 3
            pos + math::float3(-h, -h, h),  // 4
            pos + math::float3(h, -h, h),   // 5
            pos + math::float3(h, h, h),    // 6
            pos + math::float3(-h, h, h)    // 7
        };

        std::array<float, 24> _normals{};
        _normals.fill(0.f);

        auto v_count = _vertices.size();
        auto i_count = _indices.size();

        name = "cube";
        aabb = AABB::MinimalEnclosingAABB(_vertices.data(), v_count);
        material_index = -1;
        primitive_types = 0;
        vertex_count = v_count;
        face_count = i_count / 3;

        vertices = std::make_unique<float[]>(v_count * 3);
        indices = std::make_unique<unsigned int[]>(i_count);
        normals = std::make_unique<float[]>(v_count * 3);

        memcpy(vertices.get(), _vertices.data(), v_count * 3 * sizeof(float));
        memcpy(indices.get(), _indices.data(), i_count * sizeof(unsigned int));
        memcpy(normals.get(), _normals.data(), v_count * 3 * sizeof(float));

    }
};
