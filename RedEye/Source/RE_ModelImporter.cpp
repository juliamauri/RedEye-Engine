#include "RE_ModelImporter.h"

#include "Application.h"
#include "FileSystem.h"
#include "ResourceManager.h"
#include "RE_TextureImporter.h"
#include "ShaderManager.h"

#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_CompMesh.h"

#include "RE_Prefab.h"
#include "RE_Mesh.h"
#include "RE_Material.h"

#include "OutputLog.h"
#include "Globals.h"
#include "md5.h"
#include "TimeManager.h"

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

RE_ModelImporter::RE_ModelImporter(const char* f) : folderPath(f)
{
}


RE_ModelImporter::~RE_ModelImporter()
{
}

bool RE_ModelImporter::Init(const char * def_shader)
{
	LOG("Initializing Model Importer");

	App->ReportSoftware("Assimp", "4.0.1", "http://www.assimp.org/");

	return true;
}

RE_Prefab* RE_ModelImporter::LoadModelFromAssets(const char * path)
{
	Timer timer;
	LOG("Importing Model from: %s", path);
	RE_Prefab* modelreturn = nullptr;
	RE_FileIO* mesh_file = nullptr;

	std::string assetsPath(path);
	mesh_file = new RE_FileIO(assetsPath.c_str());
	if (!mesh_file->Load())
	{
		LOG_ERROR("Error when load mesh file:\n%s", assetsPath.c_str());
		DEL(mesh_file);
	}

	if (mesh_file != nullptr)
	{
		LOG("Loading Model");
		aditionalData = new currentlyImporting();
		aditionalData->workingfilepath = assetsPath;
		uint l;
		aditionalData->name = assetsPath.substr(l = assetsPath.find_last_of("/") + 1, assetsPath.find_last_of(".") - l);
		modelreturn = ProcessModel(mesh_file->GetBuffer(), mesh_file->GetSize());
		DEL(aditionalData);
		DEL(mesh_file);
		LOG("Time imported model from assets: %u ms\n", timer.Read());
	}

	return modelreturn;
}

RE_Prefab*  RE_ModelImporter::ProcessModel(const char * buffer, unsigned int size)
{
	RE_Prefab* newModelPrefab = nullptr;

	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFileFromMemory(buffer, size, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | /*aiProcess_PreTransformVertices |*/ aiProcess_SortByPType | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		LOG_ERROR("ASSIMP couldn't import file from memory! Assimp error: %s", importer.GetErrorString());
	else
	{
		LOG_SECONDARY("Loading all materials");
		//First loading all materials
		if(scene->HasMaterials()) ProcessMaterials(scene);

		LOG_SECONDARY("Loading all meshes");
		//Second loading all meshes
		if (scene->HasMeshes()) ProcessMeshes(scene);

		LOG_SECONDARY("Processing model hierarchy");
		//Mount a go hiteracy with nodes from model
		RE_GameObject* rootGO = new RE_GameObject(aditionalData->name.c_str(), GUID_NULL);
		ProcessNode(scene->mRootNode, scene, rootGO, math::float4x4::identity, true);

		LOG_SECONDARY("Saving model as internal prefab.");
		//We save own format of model as internal prefab
		newModelPrefab = new RE_Prefab(rootGO, true);
		//App->resources->Reference(newModelPrefab);
	}
	return newModelPrefab;
}

void RE_ModelImporter::ProcessNode(aiNode * node, const aiScene * scene, RE_GameObject* currentGO, math::float4x4 transform, bool isRoot)
{
	LOG_TERCIARY("%s Node: %s (%u meshes | %u children)",
		node->mParent ? "SON" : "PARENT",
		node->mName.C_Str(),
		node->mNumMeshes,
		node->mNumChildren);

	//TODO ignore child motion of warrior, we don´t want to load a skeleton.

	aiVector3D nScale;
	aiVector3D nPosition;
	aiQuaternion nRotationQuat;

	node->mTransformation.Decompose(nScale, nRotationQuat, nPosition);
	transform = transform * math::float4x4::FromTRS(
		{ nPosition.x,nPosition.y, nPosition.z },
		math::Quat(nRotationQuat.x, nRotationQuat.y, nRotationQuat.z, nRotationQuat.w),
		{ nScale.x,nScale.y, nScale.z }
	);

	RE_GameObject* go_haschildren = nullptr;
	if (node->mNumChildren > 0 || (node->mNumChildren == 0 && node->mNumMeshes == 0))
	{
		if (isRoot || std::string(node->mName.C_Str()).find("_$Assimp") == std::string::npos)
		{
			go_haschildren = (!isRoot) ? new RE_GameObject(node->mName.C_Str(), GUID_NULL, currentGO) : currentGO;

			math::float3 position;
			math::Quat rotation;
			math::float3 scale;
			transform.Decompose(position, rotation, scale);

			go_haschildren->GetTransform()->SetRotation(rotation);
			go_haschildren->GetTransform()->SetPosition(position);
			go_haschildren->GetTransform()->SetScale(scale);
			go_haschildren->GetTransform()->Update();

			transform = math::float4x4::identity;
		}
		else 		
			go_haschildren = currentGO;
	}

	unsigned int i = 0;

	if (node->mNumMeshes > 0)
	{
		for (; i < node->mNumMeshes; i++)
		{
			RE_GameObject* goMesh = (go_haschildren == nullptr) ? new RE_GameObject(node->mName.C_Str(), GUID_NULL, currentGO) : go_haschildren;
			if (go_haschildren == nullptr)
			{
				math::float3 position;
				math::Quat rotation;
				math::float3 scale;
				transform.Decompose(position, rotation, scale);

				goMesh->GetTransform()->SetRotation(rotation);
				goMesh->GetTransform()->SetPosition(position);
				goMesh->GetTransform()->SetScale(scale);
				goMesh->GetTransform()->Update();

				transform = math::float4x4::identity;
			}

			const char* md5Mesh = aditionalData->meshesLoaded.at(scene->mMeshes[node->mMeshes[i]]);
			RE_CompMesh* comp_mesh = new RE_CompMesh(goMesh, md5Mesh);
			comp_mesh->SetMaterial(aditionalData->materialsLoaded.at(scene->mMaterials[scene->mMeshes[node->mMeshes[i]]->mMaterialIndex]));
			goMesh->AddCompMesh(comp_mesh);
			
			//meshes.rbegin()->name = node->mName.C_Str();
			//total_triangle_count += meshes.rbegin()->triangle_count;
		}
	}

	for (i = 0; i < node->mNumChildren; i++)
		ProcessNode(node->mChildren[i], scene, go_haschildren, transform);
}

void RE_ModelImporter::ProcessMeshes(const aiScene* scene)
{
	for (uint i = 0; i < scene->mNumMeshes; i++) {

		aiMesh* mesh = scene->mMeshes[i];

		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		LOG_TERCIARY("Mesh %u: %s (%u vertices | %u faces | %u material index)",
			i,
			mesh->mName.C_Str(),
			mesh->mNumVertices,
			mesh->mNumFaces,
			mesh->mMaterialIndex);

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

			RE_Mesh* meshResource = new RE_Mesh(vertices, indices, aditionalData->materialsLoaded[scene->mMaterials[mesh->mMaterialIndex]], mesh->mNumFaces);
			meshResource->SetType(Resource_Type::R_MESH);
			meshResource->SetMD5(meshMD5.c_str());
			meshResource->SetLibraryPath(aditionalData->workingfilepath.c_str());

			App->resources->Reference(meshResource);
			exists = meshResource->GetMD5();
		}
		else
			LOG("Getting %s from resources, already exits\nmd5: %s\n", mesh->mName.C_Str(), exists);

		aditionalData->meshesLoaded.insert(std::pair<aiMesh*, const char*>(mesh, exists));
	}
}

const char* RE_ModelImporter::ProcessMeshFromLibrary(const char * file_library, const char * reference, const char * file_assets)
{
	Timer timer;
	LOG("Processing mesh from library:\nLibrary path: %s\nOrigin asset: %s\n", file_library, file_assets);
	std::vector<Vertex> vertexes;
	std::vector<unsigned int> indexes;

	Config mesh_serialized(file_library, App->fs->GetZipPath());
	ResourceContainer* mesh_resource = nullptr;
	if (mesh_serialized.Load())
	{
		JSONNode* mesh_json = mesh_serialized.GetRootNode("mesh");
		mesh_json->PullMeshVertex(&vertexes, &indexes);

		RE_Mesh* mesh = new RE_Mesh(vertexes, indexes, nullptr, indexes.size() / 3);
		mesh_resource = (ResourceContainer*)mesh;
		mesh_resource->SetType(Resource_Type::R_MESH);
		mesh_resource->SetMD5(reference);
		mesh_resource->SetLibraryPath(file_assets);
		App->resources->Reference(mesh_resource);

		DEL(mesh_json);
		LOG("Time imported mesh from library: %u ms\n", timer.Read());
	}
	else {
		LOG_ERROR("Can't load mesh from library. Path: %s", file_library);
	}
	return (mesh_resource) ? mesh_resource->GetMD5() : nullptr;
}

std::vector<std::string> RE_ModelImporter::GetOutsideResourcesAssetsPath(const char * path)
{
	std::vector<std::string> retPaths;
	RE_FileIO* fbxloaded = App->fs->QuickBufferFromPDPath(path);

	if (fbxloaded != nullptr)
	{
		Assimp::Importer importer;
		const aiScene *scene = importer.ReadFileFromMemory(fbxloaded->GetBuffer(), fbxloaded->GetSize(), aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | /*aiProcess_PreTransformVertices |*/ aiProcess_SortByPType | aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			LOG_ERROR("ASSIMP couldn't import file from memory! Assimp error: %s", importer.GetErrorString());
		else
		{
			if (scene->HasMaterials()) {
				for (uint i = 0; i < scene->mNumMaterials; i++) {
					aiMaterial* material = scene->mMaterials[i];

					if (uint textures = material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
					{
						for (uint t = 0; t < textures; t++)
						{
							aiString texturePath;
							if (AI_SUCCESS == material->GetTexture(aiTextureType_DIFFUSE, t, &texturePath)) {
								std::string file_path(texturePath.C_Str());
								std::string filename = file_path.substr(file_path.find_last_of("\\") + 1);
								retPaths.push_back(filename);
							}
						}
					}

					if (uint textures = material->GetTextureCount(aiTextureType_SPECULAR) > 0)
					{
						for (uint t = 0; t < textures; t++)
						{
							aiString texturePath;
							if (AI_SUCCESS == material->GetTexture(aiTextureType_SPECULAR, t, &texturePath)) {
								retPaths.push_back(texturePath.C_Str());
							}
						}
					}

					if (uint textures = material->GetTextureCount(aiTextureType_AMBIENT) > 0)
					{
						for (uint t = 0; t < textures; t++)
						{
							aiString texturePath;
							if (AI_SUCCESS == material->GetTexture(aiTextureType_AMBIENT, t, &texturePath)) {
								std::string file_path(texturePath.C_Str());
								std::string filename = file_path.substr(file_path.find_last_of("\\") + 1);
								retPaths.push_back(filename);
							}
						}
					}

					if (uint textures = material->GetTextureCount(aiTextureType_EMISSIVE) > 0)
					{
						for (uint t = 0; t < textures; t++)
						{
							aiString texturePath;
							if (AI_SUCCESS == material->GetTexture(aiTextureType_EMISSIVE, t, &texturePath)) {
								std::string file_path(texturePath.C_Str());
								std::string filename = file_path.substr(file_path.find_last_of("\\") + 1);
								retPaths.push_back(filename);
							}
						}
					}

					if (uint textures = material->GetTextureCount(aiTextureType_OPACITY) > 0)
					{
						for (uint t = 0; t < textures; t++)
						{
							aiString texturePath;
							if (AI_SUCCESS == material->GetTexture(aiTextureType_OPACITY, t, &texturePath)) {
								std::string file_path(texturePath.C_Str());
								std::string filename = file_path.substr(file_path.find_last_of("\\") + 1);
								retPaths.push_back(filename);
							}
						}
					}

					if (uint textures = material->GetTextureCount(aiTextureType_SHININESS) > 0)
					{
						for (uint t = 0; t < textures; t++)
						{
							aiString texturePath;
							if (AI_SUCCESS == material->GetTexture(aiTextureType_SHININESS, t, &texturePath)) {
								std::string file_path(texturePath.C_Str());
								std::string filename = file_path.substr(file_path.find_last_of("\\") + 1);
								retPaths.push_back(filename);
							}
						}
					}

					if (uint textures = material->GetTextureCount(aiTextureType_HEIGHT) > 0)
					{
						for (uint t = 0; t < textures; t++)
						{
							aiString texturePath;
							if (AI_SUCCESS == material->GetTexture(aiTextureType_HEIGHT, t, &texturePath)) {
								std::string file_path(texturePath.C_Str());
								std::string filename = file_path.substr(file_path.find_last_of("\\") + 1);
								retPaths.push_back(filename);
							}
						}
					}

					if (uint textures = material->GetTextureCount(aiTextureType_NORMALS) > 0)
					{
						for (uint t = 0; t < textures; t++)
						{
							aiString texturePath;
							if (AI_SUCCESS == material->GetTexture(aiTextureType_NORMALS, t, &texturePath)) {
								std::string file_path(texturePath.C_Str());
								std::string filename = file_path.substr(file_path.find_last_of("\\") + 1);
								retPaths.push_back(filename);
							}
						}
					}

					if (uint textures = material->GetTextureCount(aiTextureType_REFLECTION) > 0)
					{
						for (uint t = 0; t < textures; t++)
						{
							aiString texturePath;
							if (AI_SUCCESS == material->GetTexture(aiTextureType_REFLECTION, t, &texturePath)) {
								std::string file_path(texturePath.C_Str());
								std::string filename = file_path.substr(file_path.find_last_of("\\") + 1);
								retPaths.push_back(filename);
							}
						}
					}

					if (uint textures = material->GetTextureCount(aiTextureType_UNKNOWN) > 0)
					{
						for (uint t = 0; t < textures; t++)
						{
							aiString texturePath;
							if (AI_SUCCESS == material->GetTexture(aiTextureType_UNKNOWN, t, &texturePath)) {
								std::string file_path(texturePath.C_Str());
								std::string filename = file_path.substr(file_path.find_last_of("\\") + 1);
								retPaths.push_back(filename);
							}
						}
					}

				}
			}
		}

	}

	DEL(fbxloaded);

	return retPaths;
}

void RE_ModelImporter::ProcessMaterials(const aiScene* scene)
{
	std::string fileTexturePath = aditionalData->workingfilepath.substr(0, aditionalData->workingfilepath.find_last_of("/") + 1);

	for (int i = scene->mNumMaterials - 1; i > -1; i--)
	{
		const char* materialMD5 = nullptr;
		aiMaterial* material = scene->mMaterials[i];
		aiString name;
		if (AI_SUCCESS != material->Get(AI_MATKEY_NAME, name))
		{
			int i = 0;
		}
		LOG_TERCIARY("Loadinig %s material.", name.C_Str());

		std::string filePath("Assets/Materials/");
		filePath += name.C_Str();
		filePath += ".pupil";

		if (!App->fs->Exists(filePath.c_str())) {

			RE_Material* newMaterial = new RE_Material(name.C_Str());

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
					if (AI_SUCCESS == material->GetTexture(aiTextureType_DIFFUSE, t, &texturePath)) {
						std::string file_path(texturePath.C_Str());
						std::string filename = file_path.substr(file_path.find_last_of("\\") + 1);
						std::string realAssetsPath(fileTexturePath);
						realAssetsPath += filename;
						if (App->fs->Exists(realAssetsPath.c_str()))
						{
							const char* texture = App->textures->LoadTextureAssets(realAssetsPath.c_str());
							if (texture != nullptr) {
								newMaterial->tDiffuse.push_back(texture);
							}
							else {
								LOG_ERROR("Cannot load texture from material.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
							}
						}
						else {
							LOG_ERROR("Texture don't exists on assets.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
						}
					}
				}
			}

			aiColor3D colorDiffuse(0, 0, 0);
			if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, colorDiffuse))
			{
				if (shadingType == aiShadingMode::aiShadingMode_Flat && !colorDiffuse.IsBlack()) shadingType = aiShadingMode::aiShadingMode_Gouraud;
				newMaterial->cDiffuse.Set(colorDiffuse.r, colorDiffuse.g, colorDiffuse.g);
			}

			if (uint textures = material->GetTextureCount(aiTextureType_SPECULAR) > 0)
			{
				for (uint t = 0; t < textures; t++)
				{
					aiString texturePath;
					if (AI_SUCCESS == material->GetTexture(aiTextureType_SPECULAR, t, &texturePath)) {
						std::string file_path(texturePath.C_Str());
						std::string filename = file_path.substr(file_path.find_last_of("\\") + 1);
						std::string realAssetsPath(fileTexturePath);
						realAssetsPath += filename;
						if (App->fs->Exists(realAssetsPath.c_str()))
						{
							const char* texture = App->textures->LoadTextureAssets(realAssetsPath.c_str());
							if (texture != nullptr) {
								newMaterial->tSpecular.push_back(texture);
							}
							else {
								LOG_ERROR("Cannot load texture from material.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
							}
						}
						else {
							LOG_ERROR("Texture don't exists on assets.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
						}
					}
				}
			}

			aiColor3D colorSpecular(0, 0, 0);
			if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_SPECULAR, colorSpecular))
			{
				newMaterial->cSpecular.Set(colorSpecular.r, colorSpecular.g, colorSpecular.g);
			}

			if (uint textures = material->GetTextureCount(aiTextureType_AMBIENT) > 0)
			{
				for (uint t = 0; t < textures; t++)
				{
					aiString texturePath;
					if (AI_SUCCESS == material->GetTexture(aiTextureType_AMBIENT, t, &texturePath)) {
						std::string file_path(texturePath.C_Str());
						std::string filename = file_path.substr(file_path.find_last_of("\\") + 1);
						std::string realAssetsPath(fileTexturePath);
						realAssetsPath += filename;
						if (App->fs->Exists(realAssetsPath.c_str()))
						{
							const char* texture = App->textures->LoadTextureAssets(realAssetsPath.c_str());
							if (texture != nullptr) {
								newMaterial->tAmbient.push_back(texture);
							}
							else {
								LOG_ERROR("Cannot load texture from material.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
							}
						}
						else {
							LOG_ERROR("Texture don't exists on assets.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
						}
					}
				}
			}

			aiColor3D colorAmbient(0, 0, 0);
			if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_AMBIENT, colorAmbient))
			{
				newMaterial->cAmbient.Set(colorAmbient.r, colorAmbient.g, colorAmbient.g);
			}

			if (uint textures = material->GetTextureCount(aiTextureType_EMISSIVE) > 0)
			{
				for (uint t = 0; t < textures; t++)
				{
					aiString texturePath;
					if (AI_SUCCESS == material->GetTexture(aiTextureType_EMISSIVE, t, &texturePath)) {
						std::string file_path(texturePath.C_Str());
						std::string filename = file_path.substr(file_path.find_last_of("\\") + 1);
						std::string realAssetsPath(fileTexturePath);
						realAssetsPath += filename;
						if (App->fs->Exists(realAssetsPath.c_str()))
						{
							const char* texture = App->textures->LoadTextureAssets(realAssetsPath.c_str());
							if (texture != nullptr) {
								newMaterial->tEmissive.push_back(texture);
							}
							else {
								LOG_ERROR("Cannot load texture from material.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
							}
						}
						else {
							LOG_ERROR("Texture don't exists on assets.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
						}
					}
				}
			}

			aiColor3D colorEmissive(0, 0, 0);
			if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_EMISSIVE, colorEmissive))
			{
				newMaterial->cEmissive.Set(colorEmissive.r, colorEmissive.g, colorEmissive.g);
			}

			aiColor3D colorTransparent(0, 0, 0);
			if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_TRANSPARENT, colorTransparent))
			{
				newMaterial->cTransparent.Set(colorTransparent.r, colorTransparent.g, colorTransparent.g);
			}

			bool twosided = false;
			if (AI_SUCCESS == material->Get(AI_MATKEY_TWOSIDED, twosided))
			{
				newMaterial->backFaceCulling = !twosided;
			}

			bool blendFunc = false;
			if (AI_SUCCESS == material->Get(AI_MATKEY_BLEND_FUNC, blendFunc))
			{
				newMaterial->blendMode = blendFunc;
			}

			if (uint textures = material->GetTextureCount(aiTextureType_OPACITY) > 0)
			{
				for (uint t = 0; t < textures; t++)
				{
					aiString texturePath;
					if (AI_SUCCESS == material->GetTexture(aiTextureType_OPACITY, t, &texturePath)) {
						std::string file_path(texturePath.C_Str());
						std::string filename = file_path.substr(file_path.find_last_of("\\") + 1);
						std::string realAssetsPath(fileTexturePath);
						realAssetsPath += filename;
						if (App->fs->Exists(realAssetsPath.c_str()))
						{
							const char* texture = App->textures->LoadTextureAssets(realAssetsPath.c_str());
							if (texture != nullptr) {
								newMaterial->tOpacity.push_back(texture);
							}
							else {
								LOG_ERROR("Cannot load texture from material.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
							}
						}
						else {
							LOG_ERROR("Texture don't exists on assets.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
						}
					}
				}
			}

			float opacity = 1.0f;
			if (AI_SUCCESS == material->Get(AI_MATKEY_OPACITY, opacity))
			{
				newMaterial->opacity = opacity;
			}

			if (uint textures = material->GetTextureCount(aiTextureType_SHININESS) > 0)
			{
				for (uint t = 0; t < textures; t++)
				{
					aiString texturePath;
					if (AI_SUCCESS == material->GetTexture(aiTextureType_SHININESS, t, &texturePath)) {
						std::string file_path(texturePath.C_Str());
						std::string filename = file_path.substr(file_path.find_last_of("\\") + 1);
						std::string realAssetsPath(fileTexturePath);
						realAssetsPath += filename;
						if (App->fs->Exists(realAssetsPath.c_str()))
						{
							const char* texture = App->textures->LoadTextureAssets(realAssetsPath.c_str());
							if (texture != nullptr) {
								newMaterial->tShininess.push_back(texture);
							}
							else {
								LOG_ERROR("Cannot load texture from material.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
							}
						}
						else {
							LOG_ERROR("Texture don't exists on assets.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
						}
					}
				}
			}

			float shininess = 0.f;
			if (AI_SUCCESS == material->Get(AI_MATKEY_SHININESS, shininess))
			{
				newMaterial->shininess = shininess;
			}

			float shininessStrenght = 1.0f;
			if (AI_SUCCESS == material->Get(AI_MATKEY_SHININESS_STRENGTH, shininessStrenght))
			{
				newMaterial->shininessStrenght = shininessStrenght;
			}

			float refracti = 1.0f;
			if (AI_SUCCESS == material->Get(AI_MATKEY_REFRACTI, refracti))
			{
				newMaterial->refraccti = refracti;
			}

			if (uint textures = material->GetTextureCount(aiTextureType_HEIGHT) > 0)
			{
				for (uint t = 0; t < textures; t++)
				{
					aiString texturePath;
					if (AI_SUCCESS == material->GetTexture(aiTextureType_HEIGHT, t, &texturePath)) {
						std::string file_path(texturePath.C_Str());
						std::string filename = file_path.substr(file_path.find_last_of("\\") + 1);
						std::string realAssetsPath(fileTexturePath);
						realAssetsPath += filename;
						if (App->fs->Exists(realAssetsPath.c_str()))
						{
							const char* texture = App->textures->LoadTextureAssets(realAssetsPath.c_str());
							if (texture != nullptr) {
								newMaterial->tHeight.push_back(texture);
							}
							else {
								LOG_ERROR("Cannot load texture from material.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
							}
						}
						else {
							LOG_ERROR("Texture don't exists on assets.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
						}
					}
				}
			}

			if (uint textures = material->GetTextureCount(aiTextureType_NORMALS) > 0)
			{
				for (uint t = 0; t < textures; t++)
				{
					aiString texturePath;
					if (AI_SUCCESS == material->GetTexture(aiTextureType_NORMALS, t, &texturePath)) {
						std::string file_path(texturePath.C_Str());
						std::string filename = file_path.substr(file_path.find_last_of("\\") + 1);
						std::string realAssetsPath(fileTexturePath);
						realAssetsPath += filename;
						if (App->fs->Exists(realAssetsPath.c_str()))
						{
							const char* texture = App->textures->LoadTextureAssets(realAssetsPath.c_str());
							if (texture != nullptr) {
								newMaterial->tNormals.push_back(texture);
							}
							else {
								LOG_ERROR("Cannot load texture from material.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
							}
						}
						else {
							LOG_ERROR("Texture don't exists on assets.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
						}
					}
				}
			}

			if (uint textures = material->GetTextureCount(aiTextureType_REFLECTION) > 0)
			{
				for (uint t = 0; t < textures; t++)
				{
					aiString texturePath;
					if (AI_SUCCESS == material->GetTexture(aiTextureType_REFLECTION, t, &texturePath)) {
						std::string file_path(texturePath.C_Str());
						std::string filename = file_path.substr(file_path.find_last_of("\\") + 1);
						std::string realAssetsPath(fileTexturePath);
						realAssetsPath += filename;
						if (App->fs->Exists(realAssetsPath.c_str()))
						{
							const char* texture = App->textures->LoadTextureAssets(realAssetsPath.c_str());
							if (texture != nullptr) {
								newMaterial->tReflection.push_back(texture);
							}
							else {
								LOG_ERROR("Cannot load texture from material.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
							}
						}
						else {
							LOG_ERROR("Texture don't exists on assets.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
						}
					}
				}
			}

			if (uint textures = material->GetTextureCount(aiTextureType_UNKNOWN) > 0)
			{
				for (uint t = 0; t < textures; t++)
				{
					aiString texturePath;
					if (AI_SUCCESS == material->GetTexture(aiTextureType_UNKNOWN, t, &texturePath)) {
						std::string file_path(texturePath.C_Str());
						std::string filename = file_path.substr(file_path.find_last_of("\\") + 1);
						std::string realAssetsPath(fileTexturePath);
						realAssetsPath += filename;
						if (App->fs->Exists(realAssetsPath.c_str()))
						{
							const char* texture = App->textures->LoadTextureAssets(realAssetsPath.c_str());
							if (texture != nullptr) {
								newMaterial->tUnknown.push_back(texture);
							}
							else {
								LOG_ERROR("Cannot load texture from material.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
							}
						}
						else {
							LOG_ERROR("Texture don't exists on assets.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
						}
					}
				}
			}

			Config materialSerialize(filePath.c_str(), App->fs->GetZipPath());

			JSONNode* materialNode = materialSerialize.GetRootNode("Material");
			materialNode->SetArray();
			newMaterial->Serialize(materialNode, &materialNode->GetDocument()->FindMember("Material")->value);

			((ResourceContainer*)newMaterial)->SetFilePath(filePath.c_str());
			((ResourceContainer*)newMaterial)->SetMD5(materialSerialize.GetMd5().c_str());
			((ResourceContainer*)newMaterial)->SetType(Resource_Type::R_MATERIAL);

			materialSerialize.Save();

			materialMD5 = App->resources->Reference((ResourceContainer*)newMaterial);
		}
		else
		{
			if (!aditionalData->materialsLoaded.empty())
			{
				for (std::map<aiMaterial*, const char*>::iterator it = aditionalData->materialsLoaded.begin(); it != aditionalData->materialsLoaded.end(); ++it)
				{
					ResourceContainer* res = App->resources->At(it->second);
					if (res && std::strcmp(filePath.c_str(), res->GetFilePath()) == 0)
					{
						materialMD5 = res->GetMD5();
						break;
					}
				}
			}

			if (materialMD5 == nullptr) {
				materialMD5 = App->resources->CheckFileLoaded(filePath.c_str(), nullptr, Resource_Type::R_MATERIAL);
			}
		}

		aditionalData->materialsLoaded.insert(std::pair<aiMaterial*,const char*>(material, materialMD5));
	}
}