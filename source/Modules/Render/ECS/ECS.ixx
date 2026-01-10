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

#include <chrono>
#include <iostream>

#include <map>
#include <vector>
#include <queue>
#include <execution> 

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <Math/float4x4.h>

export module ECS;

import Camera;
import Material;
import Mesh;

import Utility;
import Files;

inline math::float4x4 ToMathGeoLib(const aiMatrix4x4& m)
{
    return math::float4x4(
        m.a1, m.b1, m.c1, m.d1,
        m.a2, m.b2, m.c2, m.d2,
        m.a3, m.b3, m.c3, m.d3,
        m.a4, m.b4, m.c4, m.d4
    );
}
export struct ECS
{
    typedef unsigned int ID;

    struct MetaData
    {
        std::string name = "root";
        unsigned short level = 0;
        unsigned short child_count = 0;
    };
    std::map<ID, MetaData> metadata = {{0, {}}};

    // Hierarchy
    std::vector<std::vector<ID>> levels = {{0}};
    std::map<ID, ID> parents = {{0, 0}};
    std::map<ID, math::float4x4> local_transforms = {{0, math::float4x4::identity}};
    std::map<ID, math::float4x4> global_transforms = {{0, math::float4x4::identity}};

    unsigned short dirty_level = max_level;
    const unsigned short max_level = 65535;

    void UpdateTransform(const ID& id, const math::float4x4& transform)
    {
        auto& local_transform = local_transforms.at(id);
        if (transform.Equals(local_transform)) return;

        local_transform = transform;
        dirty_level = std::min(dirty_level, metadata.at(id).level);
    }
    void UpdateGlobalTransforms()
    {
        for (unsigned short i = dirty_level; i < levels.size(); i++)
        {
            const auto& level = levels.at(i);
            std::for_each(std::execution::par, level.begin(), level.end(), [this](const ID& id)
                          { global_transforms.at(id) = global_transforms.at(parents.at(id)) * local_transforms.at(id); });
        }

        dirty_level = max_level;
    }

    // Components
    std::map<ID, std::vector<unsigned int>> mesh_ids{};
    std::map<ID, Camera> cameras{};

    // Resources
    ID main_camera = 0;
    std::vector<Material> loaded_materials{};
    std::vector<Mesh> loaded_meshes{};

    inline auto Size() const { return local_transforms.size(); }

    void Clear()
    {
        metadata.clear();

        levels.clear();
        parents.clear();
        local_transforms.clear();
        global_transforms.clear();

        mesh_ids.clear();
        cameras.clear();

        loaded_materials.clear();
        loaded_meshes.clear();
    }

    static ID NewEntityID()
    {
        static ID id_counter = 0;
        return ++id_counter;
    }
    ID AddEntity(ID parent = 0, const std::string& name = {}, const math::float4x4& transform = math::float4x4::identity)
    {
        ID go_id = ECS::NewEntityID();

        MetaData& parent_data = metadata.at(parent);
        parent_data.child_count++;

        MetaData child_data = {name, parent_data.level + 1};
        metadata.insert({go_id, child_data});

        if (levels.size() < child_data.level) levels.push_back({go_id});

        parents.insert({go_id, parent});
        local_transforms.insert({go_id, transform});
        global_transforms.insert({go_id, global_transforms.at(parent) * transform});
        return go_id;
    }
    
    Camera& MainCamera() { return cameras.at(main_camera); }
    const Camera& MainCamera() const { return cameras.at(main_camera); }
    Camera& AddCamera(ID go = 0)
    {
        if (go == 0) go = NewEntityID();
        if (main_camera == 0) main_camera = go;
        return cameras[go];
    }

    bool Import(const char* file_path,
                ID parent = 0,
                unsigned int flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
                                   aiProcess_PreTransformVertices | aiProcess_SortByPType | aiProcess_FlipUVs)
    {
        auto ms_start = std::chrono::high_resolution_clock::now();

        File file = { file_path };
        if (!file.Read()) return false;

        auto ms_read_file = std::chrono::high_resolution_clock::now();

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFileFromMemory(file.buffer.get(), file.size, flags);
        file.Clear();

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cerr << "ASSIMP couldn't import file (" << file_path
                      << ") from memory! Assimp error: " << importer.GetErrorString() << std::endl;
            return false;
        }

        auto ms_assimp_read = std::chrono::high_resolution_clock::now();

        AppendASSIMPScene(scene, parent);

        auto ms_ecs = std::chrono::high_resolution_clock::now();

        auto file_read_time = std::chrono::duration_cast<std::chrono::milliseconds>(ms_read_file - ms_start).count();
        auto assimp_read_time = std::chrono::duration_cast<std::chrono::milliseconds>(ms_assimp_read - ms_read_file).count();
        auto ecs_load_time = std::chrono::duration_cast<std::chrono::milliseconds>(ms_ecs - ms_assimp_read).count();
        auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(ms_ecs - ms_start).count();

        std::cout << "Imported scene '" << scene->mName.C_Str() << "' at: " << file_path << std::endl
                  << "\t- File Read: " << file_read_time << "ms (" << file_read_time * 100 / total_time << "%)" << std::endl
                  << "\t- Assimp Read: " << assimp_read_time << "ms (" << assimp_read_time * 100 / total_time << "%)" << std::endl
                  << "\t- ECS Load: " << ecs_load_time << "ms (" << ecs_load_time * 100 / total_time << "%)" << std::endl
                  << "\t- Total Time: " << total_time << "ms" << std::endl
                  << "\t- " << Size() << " entities" << std::endl
                  << "\t- " << loaded_materials.size() << "/" << scene->mNumMaterials << " materials" << std::endl
                  << "\t- " << loaded_meshes.size() << "/" << scene->mNumMeshes << " meshes" << std::endl
                  << "\t- " << scene->mNumTextures << " textures" << std::endl
                  << "\t- " << scene->mNumCameras << " cameras" << std::endl
                  << "\t- " << scene->mNumLights << " lights" << std::endl
                  << "\t- " << scene->mNumAnimations << " animations" << std::endl
                  << "\t- " << scene->mNumSkeletons << " skeletons" << std::endl;

        return true;
    }

    void AppendASSIMPScene(const aiScene* scene, ID parent)
    {
        std::vector<unsigned int> material_indexes{};
        if (scene->HasMaterials())
        {
            // TODO: Check for already loaded materials
            auto prev_loaded = loaded_materials.size();
            loaded_materials.resize(prev_loaded + scene->mNumMaterials);

            for (unsigned int i = 0; i < scene->mNumMaterials; i++)
            {
                auto target_index = prev_loaded + i;
                loaded_materials[target_index].Parse(*scene->mMaterials[i]);
                material_indexes.push_back(target_index);
            }
        }

        std::vector<unsigned int> mesh_indexes{};
        if (scene->HasMeshes())
        {
            // TODO: Check for already loaded meshes
            auto prev_loaded = loaded_meshes.size();
            loaded_meshes.resize(prev_loaded + scene->mNumMeshes);
            mesh_indexes.resize(scene->mNumMeshes);

            for (unsigned int i = 0; i < scene->mNumMeshes; i++)
                loaded_meshes[mesh_indexes[i] = prev_loaded + i].Parse(*scene->mMeshes[i], material_indexes);
        }

        std::queue<std::pair<const aiNode*, unsigned int>> nodes_queue;
        nodes_queue.push({scene->mRootNode, parent});
        while (!nodes_queue.empty())
        {
            const auto& node = nodes_queue.front();
            const auto& ai_node = *node.first;

            auto go_id = AddEntity(node.second, ai_node.mName.C_Str(), ToMathGeoLib(ai_node.mTransformation));

            if (ai_node.mNumMeshes > 0)
            {
                auto& ids = mesh_ids[go_id];
                ids.resize(ai_node.mNumMeshes);
                for (int i = 0; i < ai_node.mNumMeshes; i++)
                    ids[i] = mesh_indexes[ai_node.mMeshes[i]];
            }

            for (int i = 0; i < ai_node.mNumChildren; i++)
                nodes_queue.push({ai_node.mChildren[i], go_id});

            nodes_queue.pop();
        }
    }
};
