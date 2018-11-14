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

unsigned int MeshManager::LoadMesh(const char * name, bool dropped)
{
	std::string path;
	unsigned int ret = GetLoadedMesh(name, dropped);
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
			LOG("Loading Model from: %s", path.c_str());

			//ProcesMesh(mesh_file->GetBuffer(), mesh_file->GetSize());

			DEL(mesh_file);
		}
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
		((RE_Mesh*)App->resources->At(reference))->Draw(default_shader);
}

void MeshManager::SetDefaultShader(unsigned int shader)
{
	default_shader = shader;
}

unsigned int MeshManager::AddMesh(RE_Mesh * mesh)
{
	return App->resources->Reference((ResourceContainer*)mesh);
}

void MeshManager::ProcessModel(const char* buffer, unsigned int size)
{
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFileFromMemory(buffer, size, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_FlipUVs);;
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		LOG_ERROR("ASSIMP couldn't import file from memory! Assimp error: %s", importer.GetErrorString());
	else
	{
		RE_GameObject* go = new RE_GameObject(scene->mRootNode->mName.C_Str(), to_fill);
		to_fill = go;
		aiMatrix4x4 identity;
		ProcessNode(scene->mRootNode, scene, identity);
		to_fill->SetBoundingBox(bounding_box);
	}
}

void MeshManager::ProcessNode(aiNode * node, const aiScene * scene, aiMatrix4x4 accTransform)
{
	//RE_CompMesh* compmesh = new RE_CompMesh;
	//mesh->AddComponent((RE_Component*))

	aiMatrix4x4 transform = node->mTransformation * accTransform;


	LOG_SECONDARY("%s Node: %s (%u meshes | %u children)",
		node->mParent ? "SON" : "PARENT",
		node->mName.C_Str(),
		node->mNumMeshes,
		node->mNumChildren);

	unsigned int i = 0;

	if (node->mNumMeshes > 0)
	{
		for (; i < node->mNumMeshes; i++)
		{
			RE_GameObject* go = new RE_GameObject(node->mName.C_Str(), to_fill);
			aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
			RE_CompMesh* comp_mesh = new RE_CompMesh(go, App->resources->Reference((ResourceContainer*)ProcessMesh(mesh, scene, i + 1)));
			go->AddCompMesh(comp_mesh);
			go->SetBoundingBox(bounding_box);

			aiVector3D scale;
			aiVector3D rotation;
			aiVector3D position;

			transform.Decompose(scale, rotation, position);
			
			go->GetTransform()->SetRot(math::vec(rotation.x, rotation.y, rotation.z));
			go->GetTransform()->SetPos(math::vec(position.x, position.y, position.z));
			go->GetTransform()->SetScale(math::vec(scale.x, scale.y, scale.z));

			//meshes.rbegin()->name = node->mName.C_Str();
			//total_triangle_count += meshes.rbegin()->triangle_count;
		}
	}

	for (i = 0; i < node->mNumChildren; i++)
		ProcessNode(node->mChildren[i], scene, transform);
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
		if ((vertex.Position.x = mesh->mVertices[i].x) < bounding_box.minPoint.x) bounding_box.minPoint.x = vertex.Position.x;
		else if ((vertex.Position.x = mesh->mVertices[i].x) > bounding_box.maxPoint.x) bounding_box.maxPoint.x = vertex.Position.x;
		if ((vertex.Position.y = mesh->mVertices[i].y) < bounding_box.minPoint.y) bounding_box.minPoint.y = vertex.Position.y;
		else if ((vertex.Position.y = mesh->mVertices[i].y) > bounding_box.maxPoint.y) bounding_box.maxPoint.y = vertex.Position.y;
		if ((vertex.Position.z = mesh->mVertices[i].z) < bounding_box.minPoint.z) bounding_box.minPoint.z = vertex.Position.z;
		else if ((vertex.Position.z = mesh->mVertices[i].z) > bounding_box.maxPoint.z) bounding_box.maxPoint.z = vertex.Position.x;

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

	// process material
	aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
	std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
	//std::vector<_Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
	//textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

	RE_Mesh* ret_mesh = new RE_Mesh(vertices, indices, textures, mesh->mNumFaces);
	ResourceContainer* mesh_resource = (ResourceContainer*)ret_mesh;
	mesh_resource->SetType(Resource_Type::R_MESH);

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
			texture.path = str.C_Str();
			textures.push_back(texture);
			textures_loaded.push_back(texture); // add to loaded textures
		}
	}
	return textures;
}
