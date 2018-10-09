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

MeshManager::MeshManager(const char* f) : ResourceManager(R_MESH), folderPath(f)
{}

MeshManager::~MeshManager()
{}

bool MeshManager::Init(const char* def_shader)
{
	LOG("Initializing Mesh Manager");

	bool ret = (folderPath != nullptr);
	if (ret)
	{
		ret = App->shaders->Load(def_shader, &default_shader);
		if (!ret) LOG_ERROR("Mesh Manager could not load shader: %s", def_shader);
	}
	else
	{
		LOG_ERROR("Mesh Manager could not read folder path");
	}
	
	App->ReportSoftware("Assimp"); // , version, website);

	return ret;
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

unsigned int MeshManager::GetLoadedMesh(const char * path, const bool from_drop) const
{
	// TODO search for mesh with path
	return 0;
}

void MeshManager::DrawMesh(const unsigned int reference)
{
	if (reference > 0)
		((RE_MeshContainer*)resources.at(reference).first)->Draw(default_shader);
}

unsigned int MeshManager::AddMesh(RE_MeshContainer * mesh)
{
	unsigned int ret = 1 + (resources.size() * 2);
	ResourceIter it = resources.find(ret);

	while (it != resources.end())
		it = resources.find(ret--);

	resources[ret] = { (ResourceContainer*)mesh, 1 };
	
	return ret;
}