#include "RE_ModelImporter.h"

#include "Globals.h"
#include "RE_ConsoleLog.h"
#include "RE_FileBuffer.h"

#include "Application.h"
#include "ModuleRenderer3D.h"

#include "RE_ResourceManager.h"
#include "RE_TextureImporter.h"
#include "RE_ShaderImporter.h"
#include "RE_ECS_Manager.h"

#include "RE_Model.h"
#include "RE_Mesh.h"
#include "RE_Material.h"

#include "md5.h"
#include "assimp\include\Importer.hpp"
#include "assimp\include\scene.h"
#include "assimp\include\postprocess.h"
#include "assimp\include\material.h"

#ifdef _DEBUG
	#pragma comment(lib, "assimp/libx86/assimp-vc142-mtd.lib")
#else
	#pragma comment(lib, "assimp/libx86/assimp-vc142-mt.lib")
#endif


bool RE_ModelImporter::Init()
{
	bool ret;
	RE_LOG("Initializing Model Importer");
	RE_SOFT_NVS("Assimp", "4.0.1", "http://www.assimp.org/");
	if (!(ret = (folderPath))) RE_LOG_ERROR("Model Importer could not read folder path");
	return ret;
}

RE_ECS_Manager* RE_ModelImporter::ProcessModel(const char * buffer, unsigned int size, const char* assetPath, RE_ModelSettings* mSettings)
{
	aditionalData = new currentlyImporting();
	aditionalData->settings = mSettings;
	aditionalData->workingfilepath = assetPath;
	uint l = 0;
	aditionalData->name = aditionalData->workingfilepath.substr(l = aditionalData->workingfilepath.find_last_of("/") + 1, aditionalData->workingfilepath.find_last_of(".") - l);

	RE_ECS_Manager* ret = nullptr;

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
		ProcessNodes(ret, scene->mRootNode, scene, (ret = new RE_ECS_Manager())->AddGO(aditionalData->name.c_str(), 0)->GetUID() , math::float4x4::identity);

	}

	DEL(aditionalData);
	return ret;
}

void RE_ModelImporter::ProcessNodes(RE_ECS_Manager* goPool, aiNode * parentNode, const aiScene * scene, unsigned long long parentGO, math::float4x4 parentTransform)
{
	aiVector3D nScale, nPosition;
	aiQuaternion nRotationQuat;

	// Warning Rub: better options than that aberration?
	eastl::stack<eastl::pair< aiNode*, eastl::pair<UID, math::float4x4>>> nodes;
	nodes.push({ parentNode , {parentGO, parentTransform} });
	bool isRoot = true;

	while (!nodes.empty()) {

		eastl::pair< aiNode*, eastl::pair<UID, math::float4x4>> nodeToProcess = nodes.top();
		nodes.pop();
		aiNode* node = nodeToProcess.first;
		UID pGO = nodeToProcess.second.first;
		math::float4x4 transform = nodeToProcess.second.second;

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


		UID go_haschildren = 0ull;
		if (node->mNumChildren > 0 || (node->mNumChildren == 0 && node->mNumMeshes == 0))
		{
			if (isRoot || eastl::string(node->mName.C_Str()).find("_$Assimp") == eastl::string::npos)
			{
				go_haschildren = isRoot ? pGO : goPool->AddGO(node->mName.C_Str(), pGO)->GetUID();

				math::float3 position, scale;
				math::Quat rotation;
				transform.Decompose(position, rotation, scale);

				bool paused = Event::isPaused();
				if (!paused) Event::PauseEvents();

				RE_CompTransform* t = goPool->GetGOPtr(go_haschildren)->GetTransformPtr();
				t->SetRotation(rotation);
				t->SetPosition(position);
				t->SetScale(scale);
				t->Update();
				if (!paused) Event::ResumeEvents();

				transform = math::float4x4::identity;
			}
			else go_haschildren = pGO;
		}

		unsigned int i = 0u;
		if (node->mNumMeshes > 0)
		{
			for (; i < node->mNumMeshes; i++)
			{
				RE_GameObject* goMesh = go_haschildren ? goPool->GetGOPtr(go_haschildren) : goPool->AddGO(node->mName.C_Str(), pGO);
				if (!go_haschildren)
				{
					math::float3 position, scale;
					math::Quat rotation;
					transform.Decompose(position, rotation, scale);

					bool paused = Event::isPaused();
					if (!paused) Event::PauseEvents();

					RE_CompTransform* t = goMesh->GetTransformPtr();
					t->SetRotation(rotation);
					t->SetPosition(position);
					t->SetScale(scale);
					t->Update();

					if (!paused) Event::ResumeEvents();

					transform = math::float4x4::identity;
				}

				const char* md5Mesh = aditionalData->meshesLoaded.at(scene->mMeshes[node->mMeshes[i]]);
				RE_CompMesh* comp_mesh = dynamic_cast<RE_CompMesh*>(goMesh->AddNewComponent(C_MESH));
				comp_mesh->SetMesh(md5Mesh);
				goMesh->ResetBoundingBoxes();
				comp_mesh->SetMaterial(aditionalData->materialsLoaded.at(scene->mMaterials[scene->mMeshes[node->mMeshes[i]]->mMaterialIndex]));
			}
		}

		for (i = 0; i < node->mNumChildren; i++) nodes.push({ node->mChildren[i] ,{ go_haschildren, transform } });

		isRoot = false;
	}
}

void RE_ModelImporter::ProcessMeshes(const aiScene* scene)
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
			uint numVertices = mesh->mNumVertices;
			float* verticesArray = new float[numVertices * 3];
			memcpy(verticesArray, &mesh->mVertices[0].x, numVertices * 3 * sizeof(float));

			float* normalsArray = nullptr;
			float* tangentsArray = nullptr;
			float* bitangentsArray = nullptr;
			float* textureCoordsArray = nullptr;
			uint* indexArray = nullptr;

			if (mesh->HasNormals())
			{
				normalsArray = new float[numVertices * 3u];
				memcpy(normalsArray, &mesh->mNormals[0].x, numVertices * 3u * sizeof(float));
			}

			if (mesh->HasTangentsAndBitangents())
			{
				tangentsArray = new float[numVertices * 3u];
				memcpy(tangentsArray, &mesh->mTangents[0].x, numVertices * 3u * sizeof(float));

				bitangentsArray = new float[mesh->mNumVertices * 3u];
				memcpy(bitangentsArray, &mesh->mBitangents[0].x, numVertices * 3u * sizeof(float));
			}

			if (mesh->mTextureCoords[0])
			{
				float* cursor = (textureCoordsArray = new float[numVertices * 2u]);
				for (uint i = 0; i < numVertices; i++)
				{
					memcpy(cursor, &mesh->mTextureCoords[0][i].x, 2u * sizeof(float));
					cursor += 2u;
				}
			}

			if (mesh->HasFaces())
			{
				indexArray = new uint[mesh->mNumFaces * 3];
				uint* cursor = indexArray;

				for (unsigned int i = 0; i < mesh->mNumFaces; i++)
				{
					aiFace* face = &mesh->mFaces[i];
					memcpy(cursor, &face->mIndices[0], 3u * sizeof(uint));
					cursor += 3u;

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
				newMesh->SetType(Resource_Type::R_MESH);
				newMesh->SetAssetPath(aditionalData->workingfilepath.c_str());
				App::resources->Reference(newMesh);
			}
			else DEL(newMesh);

			aditionalData->settings->libraryMeshes.push_back(meshMD5);
			aditionalData->meshesLoaded.insert(eastl::pair<aiMesh*, const char*>(mesh, meshMD5));
		}
	}
}

eastl::vector<eastl::string> RE_ModelImporter::GetOutsideResourcesAssetsPath(const char * path)
{
	eastl::vector<eastl::string> retPaths;
	RE_FileBuffer* fbxloaded = App::fs->QuickBufferFromPDPath(path);

	if (fbxloaded)
	{
		Assimp::Importer importer;
		const aiScene *scene = importer.ReadFileFromMemory(fbxloaded->GetBuffer(), fbxloaded->GetSize(), aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | /*aiProcess_PreTransformVertices |*/ aiProcess_SortByPType | aiProcess_FlipUVs);

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

		DEL(fbxloaded);
	}
	return retPaths;
}

void RE_ModelImporter::GetTexturePath(aiMaterial * material, eastl::vector<eastl::string> &retPaths, aiTextureType textureType)
{
	if (uint textures = material->GetTextureCount(textureType) > 0)
	{
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
}

void RE_ModelImporter::ProcessMaterials(const aiScene* scene)
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
		materialMD5 = App::resources->FindMD5ByAssetsPath(filePath.c_str(), Resource_Type::R_MATERIAL);
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
			container->SetType(Resource_Type::R_MATERIAL);

			newMaterial->Save();
			materialMD5 = App::resources->Reference(container);
			App::renderer3d->PushThumnailRend(materialMD5);
		}
		aditionalData->materialsLoaded.insert(eastl::pair<aiMaterial*,const char*>(material, materialMD5));
	}
}

void RE_ModelImporter::GetTexturesMaterial(aiMaterial * material, eastl::string &fileTexturePath, aiTextureType textureType, eastl::vector<const char*>* vectorToFill, aiString &name)
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

				if (App::fs->Exists(realAssetsPath.c_str()))
				{
					const char* texture = App::resources->FindMD5ByAssetsPath(realAssetsPath.c_str(), Resource_Type::R_TEXTURE);
					if (texture) vectorToFill->push_back(texture);
					else RE_LOG_ERROR("Cannot load texture from material.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
				}
				else RE_LOG_ERROR("Texture don't exists on assets.\nMaterial: %s\nTexture Path: %s\n", name.C_Str(), realAssetsPath.c_str());
			}
		}
	}
}
