#include "RE_ModelImporter.h"

#include "Application.h"
#include "FileSystem.h"
#include "RE_ResourceManager.h"
#include "RE_TextureImporter.h"
#include "RE_ShaderImporter.h"

#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_CompMesh.h"

#include "RE_Model.h"
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

RE_GameObject*  RE_ModelImporter::ProcessModel(const char * buffer, unsigned int size, const char* assetPath, RE_ModelSettings* mSettings)
{
	aditionalData = new currentlyImporting();
	aditionalData->settings = mSettings;
	aditionalData->workingfilepath = assetPath;
	uint l = 0;
	aditionalData->name = aditionalData->workingfilepath.substr(l = aditionalData->workingfilepath.find_last_of("/") + 1, aditionalData->workingfilepath.find_last_of(".") - l);

	RE_GameObject* ret = nullptr;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFileFromMemory(buffer, size, mSettings->GetFlags()/*aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | /*aiProcess_PreTransformVertices | aiProcess_SortByPType | aiProcess_FlipUVs */);

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
		ret = new RE_GameObject(aditionalData->name.c_str(), GUID_NULL);
		ProcessNode(scene->mRootNode, scene, ret, math::float4x4::identity, true);

	}
	DEL(aditionalData);
	return ret;
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

		LOG_TERCIARY("Mesh %u: %s (%u vertices | %u faces | %u material index)",
			i,
			mesh->mName.C_Str(),
			mesh->mNumVertices,
			mesh->mNumFaces,
			mesh->mMaterialIndex);

		if (mesh->mNumVertices > 0) {
			float* verticesArray = new float[mesh->mNumVertices * 3];
			float* normalsArray = nullptr;
			float* tangentsArray = nullptr;
			float* bitangentsArray = nullptr;
			float* textureCoordsArray = nullptr;
			uint* indexArray = nullptr;

			if (mesh->HasNormals()) {
				normalsArray = new float[mesh->mNumVertices * 3];
				memcpy(normalsArray, mesh->mNormals, mesh->mNumVertices * 3);
			}

			if (mesh->HasTangentsAndBitangents()) {
				tangentsArray = new float[mesh->mNumVertices * 3];
				memcpy(tangentsArray, mesh->mTangents, mesh->mNumVertices * 3);

				bitangentsArray = new float[mesh->mNumVertices * 3];
				memcpy(bitangentsArray, mesh->mBitangents, mesh->mNumVertices * 3);
			}

			if (mesh->mTextureCoords[0]){ // does the mesh contain texture coordinates?
				textureCoordsArray = new float[mesh->mNumVertices * 2];
				memcpy(textureCoordsArray, mesh->mTextureCoords, mesh->mNumVertices * 2);
			}

			if (mesh->HasFaces()) {
				uint indexSize = mesh->mNumFaces * 3;
				indexArray = new uint[indexSize];
				uint* cursor = indexArray;
				std::fill(indexArray, indexArray + indexSize, 0);

				for (unsigned int i = 0; i < mesh->mNumFaces; i++)
				{
					aiFace face = mesh->mFaces[i];

					if (face.mNumIndices != 3)
						LOG_WARNING("Loading geometry face with %u indexes (instead of 3)", face.mNumIndices);

					memcpy(cursor, face.mIndices, face.mNumIndices * sizeof(uint));
					cursor += sizeof(uint) * 3;
				}
			}

			bool exists = false;
			RE_Mesh* newMesh = new RE_Mesh();
			newMesh->SetVerticesAndIndex(verticesArray, indexArray, mesh->mNumFaces, textureCoordsArray, normalsArray, tangentsArray, bitangentsArray);

			const char* meshMD5 = newMesh->CheckAndSave(&exists);
			if (!exists) {
				newMesh->SetName(mesh->mName.C_Str());
				newMesh->SetType(Resource_Type::R_MESH);
				newMesh->SetAssetPath(aditionalData->workingfilepath.c_str());
				App->resources->Reference(newMesh);
			}
			else
				DEL(newMesh);

			aditionalData->settings->libraryMeshes.push_back(meshMD5);
			aditionalData->meshesLoaded.insert(std::pair<aiMesh*, const char*>(mesh, meshMD5));
		}
	}
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

					GetTexturePath(material, retPaths, aiTextureType_DIFFUSE);
					GetTexturePath(material, retPaths, aiTextureType_SPECULAR);
					GetTexturePath(material, retPaths, aiTextureType_AMBIENT);
					GetTexturePath(material, retPaths, aiTextureType_EMISSIVE);
					GetTexturePath(material, retPaths, aiTextureType_OPACITY);
					GetTexturePath(material, retPaths, aiTextureType_SHININESS);
					GetTexturePath(material, retPaths, aiTextureType_HEIGHT);
					GetTexturePath(material, retPaths, aiTextureType_NORMALS);
					GetTexturePath(material, retPaths, aiTextureType_REFLECTION);
					GetTexturePath(material, retPaths, aiTextureType_UNKNOWN);
				}
			}
		}
		DEL(fbxloaded);
	}
	return retPaths;
}

void RE_ModelImporter::GetTexturePath(aiMaterial * material, std::vector<std::string> &retPaths, aiTextureType textureType)
{
	if (uint textures = material->GetTextureCount(textureType) > 0)
	{
		for (uint t = 0; t < textures; t++)
		{
			aiString texturePath;
			if (AI_SUCCESS == material->GetTexture(textureType, t, &texturePath)) {
				std::string file_path(texturePath.C_Str());
				std::string filename = file_path.substr(file_path.find_last_of("\\") + 1);
				retPaths.push_back(filename);
			}
		}
	}
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
		filePath += aditionalData->name.c_str();
		filePath += "/";
		filePath += name.C_Str();
		filePath += ".pupil";

		if (!App->fs->Exists(filePath.c_str())) {

			RE_Material* newMaterial = new RE_Material();

			aiShadingMode shadingType = aiShadingMode::aiShadingMode_Flat;
			if (AI_SUCCESS != material->Get(AI_MATKEY_SHADING_MODEL, shadingType))
			{
				int i = 0;
			}

			GetTexturesMaterial(material, fileTexturePath, aiTextureType_DIFFUSE, &newMaterial->tDiffuse, name);
			aiColor3D colorDiffuse(0, 0, 0);
			if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, colorDiffuse))
			{
				if (shadingType == aiShadingMode::aiShadingMode_Flat && !colorDiffuse.IsBlack()) shadingType = aiShadingMode::aiShadingMode_Gouraud;
				newMaterial->cDiffuse.Set(colorDiffuse.r, colorDiffuse.g, colorDiffuse.g);
			}


			GetTexturesMaterial(material, fileTexturePath, aiTextureType_SPECULAR, &newMaterial->tSpecular, name);
			aiColor3D colorSpecular(0, 0, 0);
			if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_SPECULAR, colorSpecular))
			{
				newMaterial->cSpecular.Set(colorSpecular.r, colorSpecular.g, colorSpecular.g);
			}

			GetTexturesMaterial(material, fileTexturePath, aiTextureType_AMBIENT, &newMaterial->tAmbient, name);
			aiColor3D colorAmbient(0, 0, 0);
			if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_AMBIENT, colorAmbient))
			{
				newMaterial->cAmbient.Set(colorAmbient.r, colorAmbient.g, colorAmbient.g);
			}

			GetTexturesMaterial(material, fileTexturePath, aiTextureType_EMISSIVE, &newMaterial->tEmissive, name);
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

			GetTexturesMaterial(material, fileTexturePath, aiTextureType_OPACITY, &newMaterial->tOpacity, name);
			float opacity = 1.0f;
			if (AI_SUCCESS == material->Get(AI_MATKEY_OPACITY, opacity))
			{
				newMaterial->opacity = opacity;
			}

			GetTexturesMaterial(material, fileTexturePath, aiTextureType_SHININESS, &newMaterial->tShininess, name);
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

			GetTexturesMaterial(material, fileTexturePath, aiTextureType_HEIGHT, &newMaterial->tHeight, name);
			GetTexturesMaterial(material, fileTexturePath, aiTextureType_NORMALS, &newMaterial->tNormals, name);
			GetTexturesMaterial(material, fileTexturePath, aiTextureType_REFLECTION, &newMaterial->tReflection, name);
			GetTexturesMaterial(material, fileTexturePath, aiTextureType_UNKNOWN, &newMaterial->tUnknown, name);

			((ResourceContainer*)newMaterial)->SetName(name.C_Str());
			((ResourceContainer*)newMaterial)->SetAssetPath(filePath.c_str());
			((ResourceContainer*)newMaterial)->SetType(Resource_Type::R_MATERIAL);
			newMaterial->Save();
			materialMD5 = App->resources->Reference((ResourceContainer*)newMaterial);
		}
		else
			materialMD5 = App->resources->FindMD5ByAssetsPath(filePath.c_str(), Resource_Type::R_MATERIAL);

		aditionalData->materialsLoaded.insert(std::pair<aiMaterial*,const char*>(material, materialMD5));
	}
}

void RE_ModelImporter::GetTexturesMaterial(aiMaterial * material, std::string &fileTexturePath, aiTextureType textureType, std::vector<const char*>* vectorToFill, aiString &name)
{
	if (uint textures = material->GetTextureCount(textureType) > 0)
	{
		for (uint t = 0; t < textures; t++)
		{
			aiString texturePath;
			if (AI_SUCCESS == material->GetTexture(textureType, t, &texturePath)) {
				std::string file_path(texturePath.C_Str());
				std::string filename = file_path.substr(file_path.find_last_of("\\") + 1);
				std::string realAssetsPath(fileTexturePath);
				realAssetsPath += filename;
				if (App->fs->Exists(realAssetsPath.c_str()))
				{
					const char* texture = App->resources->FindMD5ByAssetsPath(realAssetsPath.c_str());
					if (texture == nullptr) {
						texture = App->textures->AddNewTextureOnResources(realAssetsPath.c_str());
					}

					if (texture != nullptr) {
						vectorToFill->push_back(texture);
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
}
