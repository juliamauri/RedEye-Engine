#ifndef __MESHMANAGER_H__
#define __MESHMANAGER_H__

#include "RE_Math.h"
#include "ResourceManager.h"
#include "assimp/include/matrix4x4.h"
#include <vector>

class RE_MeshContainer;
class RE_GameObject;
class RE_Mesh;
struct aiNode;
struct aiMesh;
struct aiScene;
struct aiMaterial;
enum aiTextureType;
struct Texture;

class MeshManager
{
public:
	MeshManager(const char* folderPath = nullptr);
	~MeshManager();

	bool Init(const char* def_shader = nullptr);
	void LoadMeshOnGameObject(RE_GameObject* parent, const char* path, const bool dropped = false);
	unsigned int LoadMesh(const char* path, const bool dropped = false);
	unsigned int GetLoadedMesh(const char* path, const bool from_drop = false) const;
	void DrawMesh(const char* reference);
	void SetDefaultShader(unsigned int shader);

private:

	void AddMesh(RE_Mesh* mesh);
	void ProcessModel(const char* buffer, unsigned int size);
	void ProcessNode(aiNode* node, const aiScene* scene, aiMatrix4x4 transform);
	RE_Mesh* ProcessMesh(aiMesh* mesh, const aiScene* scene, const unsigned int pos);
	std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

private:

	const char* folderPath = nullptr;
	unsigned int default_shader = 0;

	RE_GameObject* to_fill = nullptr;
	bool from_drop = false;
	bool error_loading = false;
	std::string exists_md5;
	std::vector<RE_Mesh*> meshes_to_fill;
	std::vector<Texture> textures_loaded;
	std::string file;
	std::string directory;
};

#endif // !__MESHMANAGER_H__