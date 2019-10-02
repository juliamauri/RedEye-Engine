#include "Modelmporter.h"

#include "Application.h"
#include "FileSystem.h"
#include "ResourceManager.h"
#include "ShaderManager.h"

#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_CompMesh.h"

#include "RE_Prefab.h"
#include "RE_Mesh.h"

#include "OutputLog.h"
#include "Globals.h"
#include "md5.h"

#include "assimp\include\Importer.hpp"
#include "assimp\include\scene.h"
#include "assimp\include\postprocess.h"

#include <Windows.h>


Modelmporter::Modelmporter(const char* f) : folderPath(f)
{
}


Modelmporter::~Modelmporter()
{
}

bool Modelmporter::Init(const char * def_shader)
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

	App->ReportSoftware("Assimp", "4.0.1", "http://www.assimp.org/"); // , version, website);

	return ret;
}

void Modelmporter::LoadModelFromAssets(const char * path)
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
		workingfilepath = path;
		ProcessModel(mesh_file->GetBuffer(), mesh_file->GetSize());

		DEL(mesh_file);
	}
}

void Modelmporter::ProcessModel(const char * buffer, unsigned int size)
{
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFileFromMemory(buffer, size, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | /*aiProcess_PreTransformVertices |*/ aiProcess_SortByPType | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		LOG_ERROR("ASSIMP couldn't import file from memory! Assimp error: %s", importer.GetErrorString());
	else
	{
		RE_GameObject* rootGO = new RE_GameObject(scene->mRootNode->mName.C_Str(), GUID_NULL);
		ProcessNode(scene->mRootNode, scene, rootGO, true);

		RE_Prefab* newModelPrefab = new RE_Prefab(rootGO, true);
		App->resources->Reference(newModelPrefab);
	}
}

void Modelmporter::ProcessNode(aiNode * node, const aiScene * scene, RE_GameObject* currentGO, bool isRoot)
{
	LOG_SECONDARY("%s Node: %s (%u meshes | %u children)",
		node->mParent ? "SON" : "PARENT",
		node->mName.C_Str(),
		node->mNumMeshes,
		node->mNumChildren);

	RE_GameObject* go_haschildren = nullptr;
	if (node->mNumChildren > 0)
	{
		go_haschildren = new RE_GameObject(node->mName.C_Str(), GUID_NULL, currentGO);

		aiVector3D scale;
		aiVector3D position;
		aiQuaternion rotationQuat;

		node->mTransformation.Decompose(scale, rotationQuat, position);

		go_haschildren->GetTransform()->SetRotation(math::Quat(rotationQuat.x, rotationQuat.y, rotationQuat.z, rotationQuat.w));
		go_haschildren->GetTransform()->SetPosition(math::vec(position.x, position.y, position.z));
		go_haschildren->GetTransform()->SetScale(math::vec(scale.x, scale.y, scale.z));
		go_haschildren->GetTransform()->Update();
	}

	unsigned int i = 0;

	if (node->mNumMeshes > 0)
	{
		for (; i < node->mNumMeshes; i++)
		{
			RE_GameObject* goMesh = (go_haschildren == nullptr) ? new RE_GameObject(node->mName.C_Str(), GUID_NULL, currentGO) : go_haschildren;
			if (go_haschildren == nullptr)
			{
				aiVector3D scale;
				aiVector3D position;
				aiQuaternion rotationQuat;

				node->mTransformation.Decompose(scale, rotationQuat, position);

				goMesh->GetTransform()->SetRotation(math::Quat(rotationQuat.x, rotationQuat.y, rotationQuat.z, rotationQuat.w));
				goMesh->GetTransform()->SetPosition(math::vec(position.x, position.y, position.z));
				goMesh->GetTransform()->SetScale(math::vec(scale.x, scale.y, scale.z));
				goMesh->GetTransform()->Update();
			}

			aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];

			RE_CompMesh* comp_mesh = nullptr;
			RE_Mesh* processed_mesh = nullptr;
			std::string exists = ProcessMesh(mesh, scene, i + 1, processed_mesh);
			if (exists.empty())
			{
				comp_mesh = new RE_CompMesh(goMesh, App->resources->Reference((ResourceContainer*)processed_mesh));
				goMesh->SetLocalBoundingBox(processed_mesh->GetAABB());
			}
			else
			{
				comp_mesh = new RE_CompMesh(goMesh, exists.c_str());
				goMesh->SetLocalBoundingBox(((RE_Mesh*)App->resources->At(exists.c_str()))->GetAABB());
			}
			goMesh->AddCompMesh(comp_mesh);

			//meshes.rbegin()->name = node->mName.C_Str();
			//total_triangle_count += meshes.rbegin()->triangle_count;
		}
	}

	for (i = 0; i < node->mNumChildren; i++)
		ProcessNode(node->mChildren[i], scene, currentGO);
}

std::string Modelmporter::ProcessMesh(aiMesh * mesh, const aiScene * scene, const unsigned int pos, RE_Mesh* toFill)
{
	std::string meshMD5;
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

	meshMD5 = md5(vertex_buffer.c_str());

	const char* exists = App->resources->IsReference(meshMD5.c_str());
	if (!exists)
	{
		// process indices
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];

			if (face.mNumIndices != 3)
				LOG_WARNING("Loading geometry face with %u indexes (instead of 3)", face.mNumIndices);

			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		std::string save_path("Library/Meshes/");
		save_path += meshMD5;
		save_path += ".red";
		Config save_mesh(save_path.c_str(), App->fs->GetZipPath());
		JSONNode* mesh_serialize = save_mesh.GetRootNode("mesh");
		mesh_serialize->SetObject();
		mesh_serialize->PushMeshVertex(vertices, indices);
		save_mesh.Save();
		DEL(mesh_serialize);

		ret_mesh = new RE_Mesh(vertices, indices, textures, mesh->mNumFaces);
		ResourceContainer* mesh_resource = (ResourceContainer*)ret_mesh;
		mesh_resource->SetType(Resource_Type::R_MESH);
		mesh_resource->SetMD5(meshMD5.c_str());
		mesh_resource->SetFilePath(workingfilepath.c_str());
	}

	return meshMD5;
}

void Modelmporter::ProcessMeshFromLibrary(const char * file_library, const char * reference, const char * file_assets)
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