#ifndef __REMODELIMPORTER_H__
#define __REMODELIMPORTER_H__

struct aiNode;
struct aiMesh;
struct aiScene;
struct aiMaterial;
class RE_Mesh;
class RE_GameObject;
class RE_Prefab;

#include <string>
#include <map>

struct currentlyImporting {
	std::map<aiMesh*, const char*> meshesLoaded;
	std::map<aiMaterial*, const char*> materialsLoaded;
	std::string workingfilepath;
};

class RE_ModelImporter
{
public:
	RE_ModelImporter(const char* f);
	~RE_ModelImporter();

	bool Init(const char* def_shader = nullptr);
	RE_Prefab* LoadModelFromAssets(const char * path);
	RE_Prefab* ProcessModel(const char* buffer, unsigned int size);
	void ProcessMaterials(const aiScene* scene);
	void ProcessMeshes(const aiScene* scene);
	void ProcessNode(aiNode* node, const aiScene* scene, RE_GameObject* currentGO, bool isRoot = false);
	void ProcessMeshFromLibrary(const char* file_library, const char* reference, const char* file_assets);

private:
	const char* folderPath = nullptr;
	currentlyImporting* aditionalData = nullptr;
};

#endif // !__REMODELIMPORTER_H__