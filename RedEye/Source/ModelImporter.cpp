#include "ModelImporter.h"

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
#include "assimp/include/material.h"


#ifdef _DEBUG
#pragma comment(lib, "assimp/libx86/assimp-vc140-mt-debug.lib")
#else
#pragma comment(lib, "assimp/libx86/assimp-vc140-mt-release.lib")
#endif

#include <Windows.h>
#include <map>


ModelImporter::ModelImporter(const char* f) : folderPath(f)
{
}


ModelImporter::~ModelImporter()
{
}

bool ModelImporter::Init(const char * def_shader)
{
	LOG("Initializing Model Importer");

	App->ReportSoftware("Assimp", "4.0.1", "http://www.assimp.org/");

	return true;
}

RE_Prefab* ModelImporter::LoadModelFromAssets(const char * path)
{
	RE_Prefab* modelreturn = nullptr;
	RE_FileIO* mesh_file = nullptr;

	std::string assetsPath = folderPath;
	assetsPath += path;
	mesh_file = new RE_FileIO(assetsPath.c_str());
	if (!mesh_file->Load())
	{
		LOG("Error when load mesh file:\n%s", assetsPath.c_str());
		DEL(mesh_file);
	}

	if (mesh_file != nullptr)
	{
		LOG("Loading Model from: %s", assetsPath.c_str());
		workingfilepath = assetsPath;
		modelreturn = ProcessModel(mesh_file->GetBuffer(), mesh_file->GetSize());

		DEL(mesh_file);
	}

	return modelreturn;
}

RE_Prefab*  ModelImporter::ProcessModel(const char * buffer, unsigned int size)
{
	RE_Prefab* newModelPrefab = nullptr;

	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFileFromMemory(buffer, size, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | /*aiProcess_PreTransformVertices |*/ aiProcess_SortByPType | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		LOG_ERROR("ASSIMP couldn't import file from memory! Assimp error: %s", importer.GetErrorString());
	else
	{
		//First loading all materials
		if(scene->HasMaterials()) ProcessMaterials(scene);

		//Mount a go hiteracy with nodes from model
		RE_GameObject* rootGO = new RE_GameObject(scene->mRootNode->mName.C_Str(), GUID_NULL);
		ProcessNode(scene->mRootNode, scene, rootGO, true);

		//We save own format of model as internal prefab
		newModelPrefab = new RE_Prefab(rootGO, true);
		App->resources->Reference(newModelPrefab);
	}
	return newModelPrefab;
}

void ModelImporter::ProcessNode(aiNode * node, const aiScene * scene, RE_GameObject* currentGO, bool isRoot)
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
			const char* existsMesh = ProcessMesh(mesh, scene, i + 1, &processed_mesh);
			if (existsMesh == nullptr)
			{
				comp_mesh = new RE_CompMesh(goMesh, App->resources->Reference((ResourceContainer*)processed_mesh));
				goMesh->SetLocalBoundingBox(processed_mesh->GetAABB());
			}
			else
			{
				comp_mesh = new RE_CompMesh(goMesh, existsMesh);
				goMesh->SetLocalBoundingBox(((RE_Mesh*)App->resources->At(existsMesh))->GetAABB());
			}
			goMesh->AddCompMesh(comp_mesh);

			//meshes.rbegin()->name = node->mName.C_Str();
			//total_triangle_count += meshes.rbegin()->triangle_count;
		}
	}

	for (i = 0; i < node->mNumChildren; i++)
		ProcessNode(node->mChildren[i], scene, currentGO);
}

const char* ModelImporter::ProcessMesh(aiMesh * mesh, const aiScene * scene, const unsigned int pos, RE_Mesh** toFill)
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

	std::string meshMD5 = md5(vertex_buffer.c_str());
	const char* exists = App->resources->IsReference(meshMD5.c_str());
	if (exists == nullptr)
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

		(*toFill) = new RE_Mesh(vertices, indices, textures, mesh->mNumFaces);
		(*toFill)->SetType(Resource_Type::R_MESH);
		(*toFill)->SetMD5(meshMD5.c_str());
		(*toFill)->SetFilePath(workingfilepath.c_str());
	}
	return exists;
}

void ModelImporter::ProcessMeshFromLibrary(const char * file_library, const char * reference, const char * file_assets)
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

void ModelImporter::ProcessMaterials(const aiScene* scene)
{
	std::map<aiMaterial*, int> indexMaterialImporter;

	for (uint i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial* material = scene->mMaterials[i];
		aiString name;
		if (AI_SUCCESS != material->Get(AI_MATKEY_NAME, name))
		{
			int i = 0;
		}

		aiShadingMode shadingType = aiShadingMode::aiShadingMode_Flat;
		if (AI_SUCCESS != material->Get(AI_MATKEY_SHADING_MODEL, shadingType))
		{
			int i = 0;
		}

		if (uint textures = material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			for (uint t = 0; t < textures; t++)
			{
				aiString texturePath;
				if (AI_SUCCESS != material->GetTexture(aiTextureType_DIFFUSE, t, &texturePath)) {
					int i = 0;
				}
				else
					int i = 0;
			}
		}

		aiColor3D colorDiffuse(0, 0, 0);
		if (AI_SUCCESS != material->Get(AI_MATKEY_COLOR_DIFFUSE, colorDiffuse))
		{
			int i = 0;
		}
		else
		{
			if (shadingType == aiShadingMode::aiShadingMode_Flat && !colorDiffuse.IsBlack()) shadingType = aiShadingMode::aiShadingMode_Gouraud;
		}

		if (uint textures = material->GetTextureCount(aiTextureType_SPECULAR) > 0)
		{
			for (uint t = 0; t < textures; t++)
			{
				aiString texturePath;
				if (AI_SUCCESS != material->GetTexture(aiTextureType_SPECULAR, t, &texturePath)) {
					int i = 0;
				}
				else
					int i = 0;
			}
		}

		aiColor3D colorSpecular(0, 0, 0);
		if (AI_SUCCESS != material->Get(AI_MATKEY_COLOR_SPECULAR, colorSpecular))
		{
			int i = 0;
		}

		if (uint textures = material->GetTextureCount(aiTextureType_AMBIENT) > 0)
		{
			for (uint t = 0; t < textures; t++)
			{
				aiString texturePath;
				if (AI_SUCCESS != material->GetTexture(aiTextureType_AMBIENT, t, &texturePath)) {
					int i = 0;
				}
				else
					int i = 0;
			}
		}

		aiColor3D colorAmbient(0, 0, 0);
		if (AI_SUCCESS != material->Get(AI_MATKEY_COLOR_AMBIENT, colorAmbient))
		{
			int i = 0;
		}

		if (uint textures = material->GetTextureCount(aiTextureType_EMISSIVE) > 0)
		{
			for (uint t = 0; t < textures; t++)
			{
				aiString texturePath;
				if (AI_SUCCESS != material->GetTexture(aiTextureType_EMISSIVE, t, &texturePath)) {
					int i = 0;
				}
				else
					int i = 0;
			}
		}

		aiColor3D colorEmissive(0, 0, 0);
		if (AI_SUCCESS != material->Get(AI_MATKEY_COLOR_EMISSIVE, colorEmissive))
		{
			int i = 0;
		}

		aiColor3D colorTransparent(0, 0, 0);
		if (AI_SUCCESS != material->Get(AI_MATKEY_COLOR_TRANSPARENT, colorTransparent))
		{
			int i = 0;
		}

		bool twosided = false;
		if (AI_SUCCESS != material->Get(AI_MATKEY_TWOSIDED, twosided))
		{
			int i = 0;
		}

		bool blendFunc = false;
		if (AI_SUCCESS != material->Get(AI_MATKEY_BLEND_FUNC, blendFunc))
		{
			int i = 0;
		}

		if (uint textures = material->GetTextureCount(aiTextureType_OPACITY) > 0)
		{
			for (uint t = 0; t < textures; t++)
			{
				aiString texturePath;
				if (AI_SUCCESS != material->GetTexture(aiTextureType_OPACITY, t, &texturePath)) {
					int i = 0;
				}
				else
					int i = 0;
			}
		}

		float opacity = 1.0f;
		if (AI_SUCCESS != material->Get(AI_MATKEY_OPACITY, opacity))
		{
			int i = 0;
		}

		if (uint textures = material->GetTextureCount(aiTextureType_SHININESS) > 0)
		{
			for (uint t = 0; t < textures; t++)
			{
				aiString texturePath;
				if (AI_SUCCESS != material->GetTexture(aiTextureType_SHININESS, t, &texturePath)) {
					int i = 0;
				}
				else
					int i = 0;
			}
		}

		float shininess = 0.f;
		if (AI_SUCCESS != material->Get(AI_MATKEY_SHININESS, shininess))
		{
			int i = 0;
		}

		float shininessStrenght = 1.0f;
		if (AI_SUCCESS != material->Get(AI_MATKEY_SHININESS_STRENGTH, shininessStrenght))
		{
			int i = 0;
		}

		float refracti = 1.0f;
		if (AI_SUCCESS != material->Get(AI_MATKEY_REFRACTI, refracti))
		{
			int i = 0;
		}

		if (uint textures = material->GetTextureCount(aiTextureType_HEIGHT) > 0)
		{
			for (uint t = 0; t < textures; t++)
			{
				aiString texturePath;
				if (AI_SUCCESS != material->GetTexture(aiTextureType_HEIGHT, t, &texturePath)) {
					int i = 0;
				}
				else
					int i = 0;
			}
		}

		if (uint textures = material->GetTextureCount(aiTextureType_NORMALS) > 0)
		{
			for (uint t = 0; t < textures; t++)
			{
				aiString texturePath;
				if (AI_SUCCESS != material->GetTexture(aiTextureType_NORMALS, t, &texturePath)) {
					int i = 0;
				}
				else
					int i = 0;
			}
		}

		if (uint textures = material->GetTextureCount(aiTextureType_REFLECTION) > 0)
		{
			for (uint t = 0; t < textures; t++)
			{
				aiString texturePath;
				if (AI_SUCCESS != material->GetTexture(aiTextureType_REFLECTION, t, &texturePath)) {
					int i = 0;
				}
				else
					int i = 0;
			}
		}

		if (uint textures = material->GetTextureCount(aiTextureType_UNKNOWN) > 0)
		{
			for (uint t = 0; t < textures; t++)
			{
				aiString texturePath;
				if (AI_SUCCESS != material->GetTexture(aiTextureType_UNKNOWN, t, &texturePath)) {
					int i = 0;
				}
				else
					int i = 0;
			}
		}

		indexMaterialImporter.insert(std::pair<aiMaterial*,int>(material, i));
	}
}
