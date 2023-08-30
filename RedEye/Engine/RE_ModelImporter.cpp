#include <MGL/Math/float4x4.h>
#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/map.h>

#include "RE_ModelImporter.h"

#include "RE_DataTypes.h"
#include "RE_Memory.h"
#include "Application.h"
#include "RE_FileSystem.h"
#include "RE_FileBuffer.h"
#include "ModuleInput.h"
#include "ModuleRenderer3D.h"
#include "RE_ResourceManager.h"
#include "RE_TextureImporter.h"
#include "RE_ShaderImporter.h"
#include "RE_ECS_Pool.h"
#include "RE_Model.h"
#include "RE_Mesh.h"
#include "RE_Material.h"

#include <MD5/md5.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

RE_ModelImporter::CurrentlyImporting* aditionalData = nullptr;

void GetTexturesMaterial(aiMaterial* material, eastl::string& fileTexturePath, aiTextureType textureType, eastl::vector<const char*>* vectorToFill, aiString& name)
{
	if (uint textures = material->GetTextureCount(textureType) > 0)
	{
		for (uint t = 0; t < textures; t++)
		{
			aiString texturePath;
			if (AI_SUCCESS == material->GetTexture(textureType, t, &texturePath))
			{
				eastl::string file_path(texturePath.C_Str());
				eastl::string filename = file_path.substr(file_path.find_last_of("\\") + 1);
				eastl::string realAssetsPath = fileTexturePath + filename;

				if (RE_FS->Exists(realAssetsPath.c_str()))
				{
					const char* texture = RE_RES->FindMD5ByAssetsPath(realAssetsPath.c_str(), ResourceType::TEXTURE);
					if (texture) vectorToFill->push_back(texture);
					else RE_LOG_ERROR("Cannot load texture from material.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
				}
				else RE_LOG_ERROR("Texture don't exists on assets.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
			}
		}
	}
}

void ProcessMaterials(const aiScene* scene)
{
	eastl::string fileTexturePath = aditionalData->workingfilepath.substr(0, aditionalData->workingfilepath.find_last_of("/") + 1);

	for (int i = scene->mNumMaterials - 1; i > -1; i--)
	{
		const char* materialMD5 = nullptr;
		aiMaterial* material = scene->mMaterials[i];
		aiString name;
		if (AI_SUCCESS != material->Get(AI_MATKEY_NAME, name)) RE_LOG_WARNING("Material name not found.");
		RE_LOG_TERCIARY("Loadinig %s material.", name.C_Str());

		eastl::string filePath = "Assets/Materials/" + aditionalData->name + "/" + name.C_Str() + ".pupil";
		materialMD5 = RE_RES->FindMD5ByAssetsPath(filePath.c_str(), ResourceType::MATERIAL);
		if (!materialMD5)
		{
			RE_Material* newMaterial = new RE_Material();
			aiShadingMode shadingType = aiShadingMode::aiShadingMode_Flat;
			if (AI_SUCCESS != material->Get(AI_MATKEY_SHADING_MODEL, shadingType))
				RE_LOG_WARNING("Material shading model not found.");

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
				newMaterial->cSpecular.Set(colorSpecular.r, colorSpecular.g, colorSpecular.g);

			GetTexturesMaterial(material, fileTexturePath, aiTextureType_AMBIENT, &newMaterial->tAmbient, name);
			aiColor3D colorAmbient(0, 0, 0);
			if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_AMBIENT, colorAmbient))
				newMaterial->cAmbient.Set(colorAmbient.r, colorAmbient.g, colorAmbient.g);

			GetTexturesMaterial(material, fileTexturePath, aiTextureType_EMISSIVE, &newMaterial->tEmissive, name);
			aiColor3D colorEmissive(0, 0, 0);
			if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_EMISSIVE, colorEmissive))
				newMaterial->cEmissive.Set(colorEmissive.r, colorEmissive.g, colorEmissive.g);

			aiColor3D colorTransparent(0, 0, 0);
			if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_TRANSPARENT, colorTransparent))
				newMaterial->cTransparent.Set(colorTransparent.r, colorTransparent.g, colorTransparent.g);

			bool twosided = false;
			if (AI_SUCCESS == material->Get(AI_MATKEY_TWOSIDED, twosided))
				newMaterial->backFaceCulling = !twosided;

			bool blendFunc = false;
			if (AI_SUCCESS == material->Get(AI_MATKEY_BLEND_FUNC, blendFunc))
				newMaterial->blendMode = blendFunc;

			GetTexturesMaterial(material, fileTexturePath, aiTextureType_OPACITY, &newMaterial->tOpacity, name);
			float opacity = 1.0f;
			if (AI_SUCCESS == material->Get(AI_MATKEY_OPACITY, opacity))
				newMaterial->opacity = opacity;

			GetTexturesMaterial(material, fileTexturePath, aiTextureType_SHININESS, &newMaterial->tShininess, name);
			float shininess = 1.f;
			if (AI_SUCCESS == material->Get(AI_MATKEY_SHININESS, shininess))
				newMaterial->shininess = shininess;

			float shininessStrenght = 1.0f;
			if (AI_SUCCESS == material->Get(AI_MATKEY_SHININESS_STRENGTH, shininessStrenght))
				newMaterial->shininessStrenght = shininessStrenght;

			float refracti = 1.0f;
			if (AI_SUCCESS == material->Get(AI_MATKEY_REFRACTI, refracti))
				newMaterial->refraccti = refracti;

			GetTexturesMaterial(material, fileTexturePath, aiTextureType_HEIGHT, &newMaterial->tHeight, name);
			GetTexturesMaterial(material, fileTexturePath, aiTextureType_NORMALS, &newMaterial->tNormals, name);
			GetTexturesMaterial(material, fileTexturePath, aiTextureType_REFLECTION, &newMaterial->tReflection, name);
			GetTexturesMaterial(material, fileTexturePath, aiTextureType_UNKNOWN, &newMaterial->tUnknown, name);

			ResourceContainer* container = static_cast<ResourceContainer*>(newMaterial);
			container->SetName(name.C_Str());
			container->SetAssetPath(filePath.c_str());
			container->SetType(ResourceType::MATERIAL);

			newMaterial->Save();
			materialMD5 = RE_RES->Reference(container);
			RE_RENDER->PushThumnailRend(materialMD5);
		}
		aditionalData->materialsLoaded.insert(eastl::pair<aiMaterial*, const char*>(material, materialMD5));
	}
}

void ProcessMeshes(const aiScene* scene)
{
	for (uint i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[i];

		RE_LOG_TERCIARY("Mesh %u: %s (%u vertices | %u faces | %u material index)",
			i,
			mesh->mName.C_Str(),
			mesh->mNumVertices,
			mesh->mNumFaces,
			mesh->mMaterialIndex);

		if (mesh->mNumVertices > 0)
		{
			size_t numVertices = static_cast<size_t>(mesh->mNumVertices);
			float* verticesArray = new float[numVertices * 3];
			memcpy(verticesArray, &mesh->mVertices[0].x, numVertices * 3 * sizeof(float));

			float* normalsArray = nullptr;
			float* tangentsArray = nullptr;
			float* bitangentsArray = nullptr;
			float* textureCoordsArray = nullptr;
			uint* indexArray = nullptr;

			if (mesh->HasNormals())
			{
				normalsArray = new float[numVertices * 3];
				memcpy(normalsArray, &mesh->mNormals[0].x, numVertices * 3 * sizeof(float));
			}

			if (mesh->HasTangentsAndBitangents())
			{
				tangentsArray = new float[numVertices * 3];
				memcpy(tangentsArray, &mesh->mTangents[0].x, numVertices * 3 * sizeof(float));

				bitangentsArray = new float[static_cast<size_t>(mesh->mNumVertices) * 3];
				memcpy(bitangentsArray, &mesh->mBitangents[0].x, numVertices * 3 * sizeof(float));
			}

			if (mesh->mTextureCoords[0])
			{
				float* cursor = (textureCoordsArray = new float[numVertices * 2]);
				for (uint i = 0; i < numVertices; i++)
				{
					memcpy(cursor, &mesh->mTextureCoords[0][i].x, 2 * sizeof(float));
					cursor += 2u;
				}
			}

			if (mesh->HasFaces())
			{
				indexArray = new uint[static_cast<size_t>(mesh->mNumFaces) * 3];
				uint* cursor = indexArray;

				for (unsigned int i = 0; i < mesh->mNumFaces; i++)
				{
					aiFace* face = &mesh->mFaces[i];
					memcpy(cursor, &face->mIndices[0], 3 * sizeof(uint));
					cursor += 3;

					if (face->mNumIndices != 3)
						RE_LOG_WARNING("Loading geometry face with %u indexes (instead of 3)", face->mNumIndices);
				}
			}

			bool exists = false;
			RE_Mesh* newMesh = new RE_Mesh();
			newMesh->SetVerticesAndIndex(verticesArray, indexArray, numVertices, mesh->mNumFaces, textureCoordsArray, normalsArray, tangentsArray, bitangentsArray);

			const char* meshMD5 = newMesh->CheckAndSave(&exists);
			if (!exists)
			{
				newMesh->SetName(mesh->mName.C_Str());
				newMesh->SetType(ResourceType::MESH);
				newMesh->SetAssetPath(aditionalData->workingfilepath.c_str());
				RE_RES->Reference(newMesh);
			}
			else DEL(newMesh)

				aditionalData->settings->libraryMeshes.push_back(meshMD5);
			aditionalData->meshesLoaded.insert(eastl::pair<aiMesh*, const char*>(mesh, meshMD5));
		}
	}
}

void ProcessNodes(RE_ECS_Pool* goPool, aiNode* parentNode, const aiScene* scene, unsigned long long parentGO, math::float4x4 parentTransform)
{
	aiVector3D nScale, nPosition;
	aiQuaternion nRotationQuat;

	eastl::stack<eastl::tuple<aiNode*, GO_UID, math::float4x4>> nodes;
	nodes.push({ parentNode , parentGO, parentTransform });
	bool isRoot = true;

	while (!nodes.empty())
	{
		aiNode* node;
		GO_UID pGO;
		math::float4x4 transform;
		eastl::tie(node, pGO, transform) = nodes.top();
		nodes.pop();

		RE_LOG_TERCIARY("%s Node: %s (%u meshes | %u children)",
			node->mParent ? "SON" : "PARENT",
			node->mName.C_Str(),
			node->mNumMeshes,
			node->mNumChildren);

		node->mTransformation.Decompose(nScale, nRotationQuat, nPosition);
		transform = transform * math::float4x4::FromTRS(
			{ nPosition.x, nPosition.y, nPosition.z },
			math::Quat(nRotationQuat.x, nRotationQuat.y, nRotationQuat.z, nRotationQuat.w),
			{ nScale.x, nScale.y, nScale.z });


		GO_UID go_haschildren = 0ull;
		if (node->mNumChildren > 0 || (node->mNumChildren == 0 && node->mNumMeshes == 0))
		{
			if (isRoot || eastl::string(node->mName.C_Str()).find("_$Assimp") == eastl::string::npos)
			{
				go_haschildren = isRoot ? pGO : goPool->AddGO(node->mName.C_Str(), pGO)->GetUID();

				math::float3 position, scale;
				math::Quat rotation;
				transform.Decompose(position, rotation, scale);

				bool paused = RE_INPUT->Paused();
				if (!paused) RE_INPUT->PauseEvents();

				RE_CompTransform* t = goPool->GetGOPtr(go_haschildren)->GetTransformPtr();
				t->SetRotation(rotation);
				t->SetPosition(position);
				t->SetScale(scale);
				t->Update();
				if (!paused) RE_INPUT->ResumeEvents();

				transform = math::float4x4::identity;
			}
			else go_haschildren = pGO;
		}

		unsigned int i = 0;
		for (; i < node->mNumMeshes; i++)
		{
			RE_GameObject* goMesh = go_haschildren ? goPool->GetGOPtr(go_haschildren) : goPool->AddGO(node->mName.C_Str(), pGO);
			if (!go_haschildren)
			{
				math::float3 position, scale;
				math::Quat rotation;
				transform.Decompose(position, rotation, scale);

				bool paused = RE_INPUT->Paused();
				if (!paused) RE_INPUT->PauseEvents();

				RE_CompTransform* t = goMesh->GetTransformPtr();
				t->SetRotation(rotation);
				t->SetPosition(position);
				t->SetScale(scale);
				t->Update();

				if (!paused) RE_INPUT->ResumeEvents();

				transform = math::float4x4::identity;
			}

			const char* md5Mesh = aditionalData->meshesLoaded.at(scene->mMeshes[node->mMeshes[i]]);
			RE_CompMesh* comp_mesh = dynamic_cast<RE_CompMesh*>(goMesh->AddNewComponent(RE_Component::Type::MESH));
			comp_mesh->SetMesh(md5Mesh);
			goMesh->ResetBoundingBoxes();
			comp_mesh->SetMaterial(aditionalData->materialsLoaded.at(scene->mMaterials[scene->mMeshes[node->mMeshes[i]]->mMaterialIndex]));
		}

		for (i = 0; i < node->mNumChildren; i++)
			nodes.push({ node->mChildren[i], go_haschildren, transform });

		isRoot = false;
	}
}

void GetTexturePath(aiMaterial* material, eastl::vector<eastl::string>& retPaths, aiTextureType textureType)
{
	uint textures = material->GetTextureCount(textureType);
	for (uint t = 0; t < textures; t++)
	{
		aiString texturePath;
		if (AI_SUCCESS == material->GetTexture(textureType, t, &texturePath))
		{
			eastl::string file_path(texturePath.C_Str());
			retPaths.push_back(file_path.substr(file_path.find_last_of("\\") + 1));
		}
	}
}

// RE_ModelImporter

eastl::vector<eastl::string> RE_ModelImporter::GetOutsideResourcesAssetsPath(const char* path)
{
	eastl::vector<eastl::string> retPaths;
	RE_FileBuffer* fbxloaded = RE_FS->QuickBufferFromPDPath(path);

	if (fbxloaded == nullptr) return retPaths;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFileFromMemory(fbxloaded->GetBuffer(), fbxloaded->GetSize(), aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | /*aiProcess_PreTransformVertices |*/ aiProcess_SortByPType | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		RE_LOG_ERROR("ASSIMP couldn't import file from memory! Assimp error: %s", importer.GetErrorString());
	}
	else if (scene->HasMaterials())
	{
		for (uint i = 0; i < scene->mNumMaterials; i++)
		{
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

	DEL(fbxloaded)

	return retPaths;
}

RE_ECS_Pool* RE_ModelImporter::ProcessModel(const char * buffer, size_t size, const char* assetPath, RE_ModelSettings* mSettings)
{
	aditionalData = new RE_ModelImporter::CurrentlyImporting();
	aditionalData->settings = mSettings;
	aditionalData->workingfilepath = assetPath;
	eastl_size_t last_slash = aditionalData->workingfilepath.find_last_of("/") + 1;
	aditionalData->name = aditionalData->workingfilepath.substr(last_slash, aditionalData->workingfilepath.find_last_of(".") - last_slash);

	RE_ECS_Pool* ret = nullptr;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFileFromMemory(buffer, size, mSettings->GetFlags()/*aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | /*aiProcess_PreTransformVertices | aiProcess_SortByPType | aiProcess_FlipUVs */);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		RE_LOG_ERROR("ASSIMP couldn't import file from memory! Assimp error: %s", importer.GetErrorString());
	}
	else
	{
		RE_LOG_SECONDARY("Loading all materials");
		if(scene->HasMaterials()) ProcessMaterials(scene);

		RE_LOG_SECONDARY("Loading all meshes");
		if (scene->HasMeshes()) ProcessMeshes(scene);

		RE_LOG_SECONDARY("Processing model hierarchy"); // Mount a go hiteracy with nodes from model
		ProcessNodes(ret, scene->mRootNode, scene, (ret = new RE_ECS_Pool())->AddGO(aditionalData->name.c_str(), 0)->GetUID() , math::float4x4::identity);
	}

	DEL(aditionalData)
	return ret;
}
