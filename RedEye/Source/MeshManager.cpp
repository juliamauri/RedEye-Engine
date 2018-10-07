#include "MeshManager.h"

#include "Application.h"
#include "FileSystem.h"
#include "ShaderManager.h"
#include "OutputLog.h"
#include "Globals.h"
#include "RE_Mesh.h"

#include "SDL2\include\SDL_assert.h"
#include "assimp\include\Importer.hpp"
#include "assimp\include\scene.h"
#include "assimp\include\postprocess.h"

#pragma comment(lib, "assimp/libx86/assimp-vc140-mt.lib")

MeshManager::MeshManager(const char * folderPath) : folderPath(folderPath)
{}

MeshManager::~MeshManager()
{
	while (!meshes.empty())
	{
		LOG("WARNING: Unreferenced mesh: %s", meshes.begin()->second.first->GetFileName());
		meshes.erase(meshes.begin());
	}
}

void MeshManager::Init(const char* def_shader)
{
	// TODO App->ReportSoftware(name, version, website); assimp;

	App->shaders->Load(def_shader, &default_shader);
}

unsigned int MeshManager::LoadMesh(const char * path, bool dropped)
{
	unsigned int ret = GetLoadedMesh(path, dropped);

	if (!ret)
	{
		RE_FileIO* mesh_file = nullptr;
		RE_FileIO mesh_file_tmp(path);

		if (dropped)
		{
			mesh_file = App->fs->QuickBufferFromPDPath(path);
		}
			
		else if (mesh_file_tmp.Load())
		{
			mesh_file = &mesh_file_tmp;
		}

		if (mesh_file != nullptr)
		{
			Assimp::Importer importer;
			const aiScene *scene = importer.ReadFileFromMemory(mesh_file->GetBuffer(), mesh_file->GetSize(), aiProcess_Triangulate | aiProcess_FlipUVs);

			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				LOG("ERROR::ASSIMP::%s", importer.GetErrorString());
			}
			else
			{
				std::string path_s(path);
				unsigned int separator_pos = path_s.find_last_of(dropped ? '\\' : '/');
				RE_MeshContainer* mesh = new RE_MeshContainer(
					path_s.substr(separator_pos, path_s.size()).c_str(),
					path_s.substr(0, separator_pos).c_str(),
					true);
				mesh->ProcessNode(scene->mRootNode, scene);
				ret = AddMesh(mesh);
			}
		}

		if (dropped) DEL(mesh_file);
	}

	return ret;
}

bool MeshManager::UnReference(unsigned int mesh_id)
{
	bool ret = false;

	if (mesh_id > 0)
	{
		MeshIter it = meshes.find(mesh_id);
		if (it != meshes.end())
		{
			it->second.second--;
			if (it->second.second == 0)
			{
				// Release Mesh
				SDL_assert(it->second.first != nullptr);
				DEL(it->second.first);
				meshes.erase(it);
			}
			ret = true;
		}
	}

	return ret;
}

RE_MeshContainer* MeshManager::operator[](unsigned int mesh_id) const
{
	MeshConstIter it = meshes.find(mesh_id);
	return it != meshes.end() ? it->second.first : nullptr;
}

unsigned int MeshManager::TotalMeshes() const
{
	return meshes.size();
}

unsigned int MeshManager::GetLoadedMesh(const char * path, const bool from_drop) const
{
	// TODO search for mesh with path
	return 0;
}

void MeshManager::DrawMesh(const unsigned int reference)
{
	if (reference > 0)
		meshes.at(reference).first->Draw(default_shader);
}

unsigned int MeshManager::AddMesh(RE_MeshContainer * mesh)
{
	unsigned int ret = 1 + (meshes.size() * 2);
	MeshIter it = meshes.find(ret);

	while (it != meshes.end())
		it = meshes.find(ret--);

	meshes[ret] = { mesh, 1 };
	
	return ret;
}