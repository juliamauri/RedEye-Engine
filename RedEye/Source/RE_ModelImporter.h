#ifndef __REMODELIMPORTER_H__
#define __REMODELIMPORTER_H__

struct aiNode;
struct aiMesh;
struct aiScene;
struct aiMaterial;
struct aiString;
struct RE_ModelSettings;
enum aiTextureType;
class RE_GameObject;

#include "MathGeoLib/include/Math/float4x4.h"

#include <vector>
#include <string>
#include <map>

struct currentlyImporting {
	RE_ModelSettings* settings = nullptr;
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

	std::vector<std::string> GetOutsideResourcesAssetsPath(const char * path);
	RE_GameObject* ProcessModel(const char* buffer, unsigned int size, const char* assetPayh, RE_ModelSettings* mSettings);

private:
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