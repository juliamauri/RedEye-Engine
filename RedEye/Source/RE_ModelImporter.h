#ifndef __REMODELIMPORTER_H__
#define __REMODELIMPORTER_H__

struct aiNode;
struct aiMesh;
struct aiScene;
struct aiMaterial;
struct aiString;
enum aiTextureType;
class RE_GameObject;
class RE_Prefab;

#include "MathGeoLib/include/Math/float4x4.h"

#include <vector>
#include <string>
#include <map>

struct currentlyImporting {
	std::map<aiMesh*, const char*> meshesLoaded;
	std::map<aiMaterial*, const char*> materialsLoaded;
	std::string workingfilepath;
	std::string name;
};

class RE_ModelImporter
{
public:
	RE_ModelImporter(const char* f);
	~RE_ModelImporter();

	bool Init(const char* def_shader = nullptr);
	RE_Prefab* LoadModelFromAssets(const char * path);
	const char* ProcessMeshFromLibrary(const char* file_library, const char* reference, const char* file_assets);

	std::vector<std::string> GetOutsideResourcesAssetsPath(const char * path);

private:
	RE_Prefab* ProcessModel(const char* buffer, unsigned int size);
	void ProcessMaterials(const aiScene* scene);
	void ProcessMeshes(const aiScene* scene);
	void ProcessNode(aiNode* node, const aiScene* scene, RE_GameObject* currentGO, math::float4x4 transform, bool isRoot = false);

	void GetTexturesMaterial(aiMaterial * material, std::string &fileTexturePath, aiTextureType textureType, std::vector<const char*>* vectorToFill, aiString &name);
	void GetTexturePath(aiMaterial * material, std::vector<std::string> &retPaths, aiTextureType textureType);

private:
	const char* folderPath = nullptr;
	currentlyImporting* aditionalData = nullptr;
};

#endif // !__REMODELIMPORTER_H__