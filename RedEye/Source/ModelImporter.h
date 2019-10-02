#ifndef __MODELIMPORTER_H__
#define __MODELIMPORTER_H__

struct aiNode;
struct aiMesh;
struct aiScene;
class RE_Mesh;
class RE_GameObject;
class RE_Prefab;

#include <string>

class ModelImporter
{
public:
	ModelImporter(const char* f);
	~ModelImporter();

	bool Init(const char* def_shader = nullptr);
	RE_Prefab* LoadModelFromAssets(const char * path);
	RE_Prefab* ProcessModel(const char* buffer, unsigned int size);
	void ProcessNode(aiNode* node, const aiScene* scene, RE_GameObject* currentGO, bool isRoot = false);
	const char* ProcessMesh(aiMesh* mesh, const aiScene* scene, const unsigned int pos, RE_Mesh** toFill);
	void ProcessMeshFromLibrary(const char* file_library, const char* reference, const char* file_assets);

private:
	const char* folderPath = nullptr;
	std::string workingfilepath;
};

#endif // !__MODELIMPORTER_H__