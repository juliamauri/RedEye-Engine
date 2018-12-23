#include "MeshManager.h"

#include "Application.h"
#include "RE_GameObject.h"
#include "FileSystem.h"
#include "ShaderManager.h"
#include "Texture2DManager.h"
#include "OutputLog.h"
#include "Globals.h"
#include "RE_CompMesh.h"
#include "RE_CompTransform.h"
#include "RE_Mesh.h"
#include "md5.h"

#include "SDL2\include\SDL_assert.h"
#include "assimp\include\Importer.hpp"
#include "assimp\include\scene.h"
#include "assimp\include\postprocess.h"

#pragma comment(lib, "assimp/libx86/assimp-vc140-mt.lib")

MeshManager::MeshManager(const char* f) : folderPath(f)
{}

MeshManager::~MeshManager()
{}

bool MeshManager::Init(const char* def_shader)
{
	LOG("Initializing Mesh Manager");

	bool ret = (folderPath != nullptr);
	if (ret)
	{
		if (def_shader != nullptr)
		{
			ret = App->shaders->Load(def_shader, &default_shader);
			if (!ret) LOG_ERROR("Mesh Manager could not load shader: %s", def_shader);
		}
	}
	else
	{
		LOG_ERROR("Mesh Manager could not read folder path");
	}
	
	App->ReportSoftware("Assimp"); // , version, website);

	return ret;
}

void MeshManager::LoadMeshOnGameObject(RE_GameObject* parent, const char * name, const bool dropped)
{
	to_fill = parent;
	from_drop = dropped;
	std::string path;
	unsigned int ret = GetLoadedMesh(name , dropped);
	if (!ret)
	{
		RE_FileIO* mesh_file = nullptr;
		if (dropped)
		{
			path += name;
			mesh_file = App->fs->QuickBufferFromPDPath(path.c_str());
			if (!mesh_file)
				LOG("Error when load mesh from dropped file:\n%s", path.c_str());
		}
		else
		{
			path += folderPath;
			path += name;
			mesh_file = new RE_FileIO(path.c_str());
			if (!mesh_file->Load())
			{
				LOG("Error when load mesh file:\n%s", path.c_str());
				DEL(mesh_file);
			}
		}

		if (mesh_file != nullptr)
		{
			std::string path_s(path.c_str());
			unsigned int separator_pos = path_s.find_last_of(dropped ? '\\' : '/');
			file = path_s.substr(separator_pos, path_s.size()).c_str();
			directory = path_s.substr(0, separator_pos).c_str();


			LOG("Loading Model from: %s", path.c_str());
			ProcessModel(mesh_file->GetBuffer(), mesh_file->GetSize());

			DEL(mesh_file);
		}
	}
}

void MeshManager::LoadMesh(const char * path, bool dropped)
{

	RE_FileIO* mesh_file = nullptr;

	mesh_file = new RE_FileIO(path);
	if (!mesh_file->Load())
	{
		LOG("Error when load mesh file:\n%s", path);
		DEL(mesh_file);
	}

	if (mesh_file != nullptr)
	{
		LOG("Loading Model from: %s", path);

		std::string path_s(path);
		unsigned int separator_pos = path_s.find_last_of(dropped ? '\\' : '/');
		file = path_s.substr(separator_pos, path_s.size()).c_str();
		directory = path_s.substr(0, separator_pos).c_str();

		ProcessModel(mesh_file->GetBuffer(), mesh_file->GetSize(), false);

		DEL(mesh_file);
	}
}

void MeshManager::LoadDirectMesh(const char* file_library, const char* reference, const char* file_assets)
{
	std::vector<Texture> null_text;
	std::vector<Vertex> vertexes;
	std::vector<unsigned int> indexes;

	Config mesh_serialized(file_library, App->fs->GetZipPath());

	if (mesh_serialized.Load())
	{
		JSONNode* mesh_json = mesh_serialized.GetRootNode("mesh");
		mesh_json->PullMeshVertex(&vertexes, &indexes);

		RE_Mesh* mesh = new RE_Mesh(vertexes, indexes, null_text, indexes.size() / 3);
		ResourceContainer* mesh_resource = (ResourceContainer*)mesh;
		mesh_resource->SetType(Resource_Type::R_MESH);
		mesh_resource->SetMD5(reference);
		mesh_resource->SetFilePath(file_assets);
		App->resources->Reference(mesh_resource);

		DEL(mesh_json);
	}
}

unsigned int MeshManager::GetLoadedMesh(const char * path, const bool from_drop) const
{
	// TODO search for mesh with path
	return 0;
}

void MeshManager::DrawMesh(const char* reference)
{
	if (reference > 0)
		((RE_Mesh*)App->resources->At(reference))->Draw(default_shader);
}

void MeshManager::DrawMesh(RE_Mesh * mesh)
{
	mesh->Draw(default_shader);
}

void MeshManager::SetDefaultShader(unsigned int shader)
{
	default_shader = shader;
}

void MeshManager::AddMesh(RE_Mesh * mesh)
{
	App->resources->Reference((ResourceContainer*)mesh);
}

void MeshManager::ProcessModel(const char* buffer, unsigned int size, bool go_fill)
{
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFileFromMemory(buffer, size, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | /*aiProcess_PreTransformVertices |*/ aiProcess_SortByPType | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		LOG_ERROR("ASSIMP couldn't import file from memory! Assimp error: %s", importer.GetErrorString());
	else
	{
		RE_GameObject* go = nullptr;

		if (go_fill)
		{
			go = new RE_GameObject(scene->mRootNode->mName.C_Str(), GUID_NULL, to_fill);
			to_fill = go;
			isFilliingGO = true;
		}
		else
			isFilliingGO = false;

		ProcessNode(scene->mRootNode, scene, true);
	}
}

void MeshManager::ProcessNode(aiNode * node, const aiScene * scene, bool isRoot)
{
	LOG_SECONDARY("%s Node: %s (%u meshes | %u children)",
		node->mParent ? "SON" : "PARENT",
		node->mName.C_Str(),
		node->mNumMeshes,
		node->mNumChildren);

	RE_GameObject* go_haschildren = nullptr;
	if (node->mNumChildren > 0)
	{
		go_haschildren = new RE_GameObject(node->mName.C_Str(), GUID_NULL, to_fill);

		aiVector3D scale;
		aiVector3D rotation;
		aiVector3D position;
		node->mTransformation.Decompose(scale, rotation, position);

		go_haschildren->GetTransform()->SetRotation(math::vec(rotation.x, rotation.y, rotation.z));
		go_haschildren->GetTransform()->SetPosition(math::vec(position.x, position.y, position.z));
		go_haschildren->GetTransform()->SetScale(math::vec(scale.x, scale.y, scale.z));
		go_haschildren->GetTransform()->Update();
		to_fill = go_haschildren;
	}

	unsigned int i = 0;

	if (node->mNumMeshes > 0)
	{
		for (; i < node->mNumMeshes; i++)
		{
			if (isFilliingGO)
			{
				RE_GameObject* go = nullptr;
				if (go_haschildren == nullptr)
					go = new RE_GameObject(node->mName.C_Str(), GUID_NULL, to_fill);
				else
					go = go_haschildren;
				
				aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];

				RE_CompMesh* comp_mesh = nullptr;
				RE_Mesh* processed_mesh = ProcessMesh(mesh, scene, i + 1);
				if (processed_mesh)
				{
					comp_mesh = new RE_CompMesh(go, App->resources->Reference((ResourceContainer*)processed_mesh));
					go->SetLocalBoundingBox(processed_mesh->GetAABB());
				}
				else
				{
					comp_mesh = new RE_CompMesh(go, exists_md5.c_str());
					go->SetLocalBoundingBox(((RE_Mesh*)App->resources->At(exists_md5.c_str()))->GetAABB());
				}

				go->AddCompMesh(comp_mesh);

				if (go_haschildren == nullptr)
				{
					aiVector3D scale;
					aiVector3D rotation;
					aiVector3D position;

					node->mTransformation.Decompose(scale, rotation, position);

					go->GetTransform()->SetRotation(math::vec(rotation.x, rotation.y, rotation.z));
					go->GetTransform()->SetPosition(math::vec(position.x, position.y, position.z));
					go->GetTransform()->SetScale(math::vec(scale.x, scale.y, scale.z));
					go->GetTransform()->Update();
				}
				//meshes.rbegin()->name = node->mName.C_Str();
				//total_triangle_count += meshes.rbegin()->triangle_count;
			}
			else
			{
				aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
				RE_Mesh* processed_mesh = ProcessMesh(mesh, scene, i + 1);
				if (processed_mesh)
					App->resources->Reference((ResourceContainer*)processed_mesh);
			}

		}
	}

	for (i = 0; i < node->mNumChildren; i++)
	{
		if(isRoot) 
			to_fill = go_haschildren;
		ProcessNode(node->mChildren[i], scene);
	}
}

RE_Mesh* MeshManager::ProcessMesh(aiMesh * mesh, const aiScene * scene, const unsigned int pos)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	LOG_TERCIARY("Mesh %u: %s (%u vertices | %u faces | %u texture indexes | %u bones)",
		pos,
		mesh->mName.C_Str(),
		mesh->mNumVertices,
		mesh->mNumFaces,
		mesh->mMaterialIndex,
		mesh->mNumBones);

	// process vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		// process vertex positions, normals and texture coordinates
		Vertex vertex;

		vertex.Position.x = mesh->mVertices[i].x;
		vertex.Position.y = mesh->mVertices[i].y;
		vertex.Position.z = mesh->mVertices[i].z;

		vertex.Normal.x = mesh->mNormals[i].x;
		vertex.Normal.y = mesh->mNormals[i].y;
		vertex.Normal.z = mesh->mNormals[i].z;

		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
			vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
		}
		else
			vertex.TexCoords = math::float2::zero;

		vertices.push_back(vertex);
	}

	std::string vertex_buffer;

	for (auto vertice : vertices)
	{
		vertex_buffer += vertice.Position.ToString();
	}

	RE_Mesh* ret_mesh = nullptr;

	std::string md5_id = md5(vertex_buffer.c_str());

	const char* exists = App->resources->IsReference(md5_id.c_str());
	if (exists)
		exists_md5 = exists;
	else
	{
		// process indices
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];

			if (face.mNumIndices != 3)
			{
				error_loading = true;
				LOG_WARNING("Loading geometry face with %u indexes (instead of 3)", face.mNumIndices);
			}

			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		if (isFilliingGO)
		{
			// process material
			aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
			std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			//std::vector<_Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			//textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		}

		
		std::string save_path("Library/Meshes/");
		save_path += md5_id;
		save_path += ".red";
		Config save_mesh(save_path.c_str(), App->fs->GetZipPath());
		JSONNode* mesh_serialize = save_mesh.GetRootNode("mesh");
		mesh_serialize->SetObject();
		mesh_serialize->PushMeshVertex(vertices, indices);
		save_mesh.Save();
		DEL(mesh_serialize);
		/*
		Load::
		std::vector<Vertex> test_ertices;
		std::vector<unsigned int> test_indices;
		mesh_serialize->PullMeshVertex(&test_ertices, &test_indices);
		*/
		ret_mesh = new RE_Mesh(vertices, indices, textures, mesh->mNumFaces);
		ResourceContainer* mesh_resource = (ResourceContainer*)ret_mesh;
		mesh_resource->SetType(Resource_Type::R_MESH);
		mesh_resource->SetMD5(md5_id.c_str());
		std::string path(directory);
		path += file;
		mesh_resource->SetFilePath(path.c_str());
	}
	return ret_mesh;
}

std::vector<Texture> MeshManager::LoadMaterialTextures(aiMaterial * mat, aiTextureType type, std::string typeName)
{
	std::vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		bool skip = false;
		for (unsigned int j = 0; j < textures_loaded.size(); j++)
		{
			if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
			{
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}
		if (!skip)
		{   // if texture hasn't been loaded already, load it
			Texture texture;
			texture.id = App->textures->LoadTexture2D(directory.c_str(), str.C_Str(), from_drop);
			texture.type = typeName;
			std::string tex_path(directory);
			tex_path += "/";
			std::string text_filename(str.C_Str());
			unsigned int separator_pos = text_filename.find_last_of('\\');
			tex_path += text_filename.substr(separator_pos + 1, text_filename.size()).c_str();
			texture.path = tex_path.c_str();
			texture.ptr = (Texture2D*)App->resources->At(texture.id.c_str());
			textures.push_back(texture);
			textures_loaded.push_back(texture); // add to loaded textures
		}
	}
	return textures;
}
