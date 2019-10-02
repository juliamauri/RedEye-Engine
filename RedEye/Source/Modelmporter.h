#ifndef __MODELIMPORTER_H__
#define __MODELIMPORTER_H__

struct aiNode;
struct aiMesh;
struct aiScene;
class RE_Mesh;
class RE_GameObject;

#include <string>

class Modelmporter
{
public:
	Modelmporter(const char* f);
	~Modelmporter();

	bool Init(const char* def_shader = nullptr);
	void LoadModelFromAssets(const char * path);
	void ProcessModel(const char* buffer, unsigned int size);
	void ProcessNode(aiNode* node, const aiScene* scene, RE_GameObject* currentGO, bool isRoot = false);
	std::string ProcessMesh(aiMesh* mesh, const aiScene* scene, const unsigned int pos, RE_Mesh* toFill);
	void ProcessMeshFromLibrary(const char* file_library, const char* reference, const char* file_assets);

private:
	const char* folderPath = nullptr;
	unsigned int default_shader = 0;
	std::string workingfilepath;
};

#endif // !__MODELIMPORTER_H__