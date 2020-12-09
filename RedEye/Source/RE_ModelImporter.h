#ifndef __REMODELIMPORTER_H__
#define __REMODELIMPORTER_H__

struct aiNode;
struct aiMesh;
struct aiScene;
struct aiMaterial;
struct aiString;
struct RE_ModelSettings;
enum aiTextureType;
class RE_ECS_Pool;
class RE_GameObject;

#include "MathGeoLib/include/Math/float4x4.h"

#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/map.h>

namespace RE_ModelImporter
{
	void Init();
	eastl::vector<eastl::string> GetOutsideResourcesAssetsPath(const char * path);
	RE_ECS_Pool* ProcessModel(const char* buffer, unsigned int size, const char* assetPayh, RE_ModelSettings* mSettings);

	namespace Internal
	{
		struct currentlyImporting {
			RE_ModelSettings* settings = nullptr;
			eastl::map<aiMesh*, const char*> meshesLoaded;
			eastl::map<aiMaterial*, const char*> materialsLoaded;
			eastl::string workingfilepath;
			eastl::string name;
		} static *aditionalData = nullptr;

		void ProcessMaterials(const aiScene* scene);
		void ProcessMeshes(const aiScene* scene);
		void ProcessNodes(RE_ECS_Pool* goPool, aiNode* parentNode, const aiScene* scene, unsigned long long parentGO, math::float4x4 patrentTansform);

		void GetTexturesMaterial(aiMaterial* material, eastl::string& fileTexturePath, aiTextureType textureType, eastl::vector<const char*>* vectorToFill, aiString& name);
		void GetTexturePath(aiMaterial* material, eastl::vector<eastl::string>& retPaths, aiTextureType textureType);
	}
};

#endif // !__REMODELIMPORTER_H__