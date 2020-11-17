#ifndef __REMODELIMPORTER_H__
#define __REMODELIMPORTER_H__

struct aiNode;
struct aiMesh;
struct aiScene;
struct aiMaterial;
struct aiString;
struct RE_ModelSettings;
enum aiTextureType;
class RE_GOManager;
class RE_GameObject;

#include "Globals.h"
#include "MathGeoLib/include/Math/float4x4.h"

#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/map.h>

struct currentlyImporting {
	RE_ModelSettings* settings = nullptr;
	eastl::map<aiMesh*, const char*> meshesLoaded;
	eastl::map<aiMaterial*, const char*> materialsLoaded;
	eastl::string workingfilepath;
	eastl::string name;
};

class RE_ModelImporter
{
public:
	RE_ModelImporter(const char* folder = "Assets/Meshes/");
	~RE_ModelImporter();

	bool Init();

	eastl::vector<eastl::string> GetOutsideResourcesAssetsPath(const char * path);
	RE_GOManager* ProcessModel(const char* buffer, unsigned int size, const char* assetPayh, RE_ModelSettings* mSettings);

private:
	void ProcessMaterials(const aiScene* scene);
	void ProcessMeshes(const aiScene* scene);
	void ProcessNode(RE_GOManager* goPool, aiNode* node, const aiScene* scene, UID currentGO, math::float4x4 transform, bool isRoot = false);

	void GetTexturesMaterial(aiMaterial * material, eastl::string &fileTexturePath, aiTextureType textureType, eastl::vector<const char*>* vectorToFill, aiString &name);
	void GetTexturePath(aiMaterial * material, eastl::vector<eastl::string> &retPaths, aiTextureType textureType);

private:
	const char* folderPath = nullptr;
	currentlyImporting* aditionalData = nullptr;
};

#endif // !__REMODELIMPORTER_H__