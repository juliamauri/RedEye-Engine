#ifndef __MESHMANAGER_H__
#define __MESHMANAGER_H__

#include "MathGeoLib/include/MathGeoLib.h"

#include <list>
#include <vector>
#include <map>

/*struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;

struct Vertex {
	// position
	math::vec Position;
	// normal
	math::vec Normal;
	// texCoords
	math::float2 TexCoords;
};

struct Texture {
	unsigned int id;
	std::string type;
	std::string path;
};

struct RE_Mesh
{
	const char* buffer_file = nullptr;
	unsigned int buffer_size = 0;
	bool droped = false;
	std::vector<RE_Mesh> meshes;
	std::string directory;
	std::vector<Texture> textures_loaded;
};

class MeshManager
{
public:
	MeshManager(const char* folderPath);
	~MeshManager();

	unsigned int LoadMesh(const char* path, bool droped = false);

	RE_Mesh* operator[](unsigned int mesh_id);

	void MeshReleased(unsigned int mesh_id);


private:

	void loadModel(std::string path);
	void processNode(aiNode *node, const aiScene *scene);
	RE_Mesh processMesh(aiMesh *mesh, const aiScene *scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);

private:

	const char* folderPath;
	unsigned int ID_count = 0;

	std::list<unsigned int> meshIDContainer;
	std::map<unsigned int, RE_Mesh*> meshes;
};*/

#endif // !__MESHMANAGER_H__