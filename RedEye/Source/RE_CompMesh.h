#ifndef __RE_COMPMESH_H__
#define __RE_COMPMESH_H__


#include "RE_Mesh.h"

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;

class RE_CompMesh
{
public:
	/*  Functions   */
	RE_CompMesh(char *path = nullptr);
	RE_CompMesh(char *path, const char* buffer, unsigned int size);

	void Draw(unsigned int shader);
protected:
	/*  Model Data  */
	const char* buffer_file = nullptr;
	unsigned int buffer_size = 0;
	bool droped = false;
	std::vector<RE_Mesh> meshes;
	std::string directory;
	std::vector<Texture> textures_loaded;
	
	/*  Functions   */
	void loadModel(std::string path);
	void processNode(aiNode *node, const aiScene *scene);
	RE_Mesh processMesh(aiMesh *mesh, const aiScene *scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
		std::string typeName);
};
#endif // !__RE_COMPMESH_H__