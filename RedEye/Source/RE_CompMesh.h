#ifndef __RE_COMPMESH_H__
#define __RE_COMPMESH_H__

#include "RE_Component.h"

class RE_CompMesh : public RE_Component
{
public:
	RE_CompMesh(RE_GameObject* go = nullptr, const char *path = nullptr, const bool file_dropped = false, const bool start_active = true);
	~RE_CompMesh();

	unsigned int LoadMesh(const char* path, const bool dropped = false);

	void Draw() override;

protected:

	unsigned int reference = 0u;
};






























#include <vector>
#include "MathGeoLib/include/MathGeoLib.h"

struct _Vertex {
	// position
	math::vec Position;
	// normal
	math::vec Normal;
	// texCoords
	math::float2 TexCoords;
	// tangent
	//math::vec Tangent;
	// bitangent
	//math::vec Bitangent;
};


struct _Texture {
	unsigned int id;
	std::string type;
	std::string path;
};

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;

class RE_UnregisteredMesh
{
public:
	/*  Mesh Data  */
	std::vector<_Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<_Texture> textures;
	unsigned int VAO;

	/*  Functions  */
	// constructor
	RE_UnregisteredMesh(std::vector<_Vertex> vertices, std::vector<unsigned int> indices, std::vector<_Texture> textures);

	// render the mesh
	void Draw(unsigned int shader_ID, bool f_normals = false, bool v_normals = false);

private:
	/*  Render data  */
	unsigned int VBO, EBO;

	/*  Functions    */
	// initializes all the buffer objects/arrays
	void _setupMesh();
};

class RE_CompUnregisteredMesh
{
public:
	/*  Functions   */
	RE_CompUnregisteredMesh(char *path);
	RE_CompUnregisteredMesh(char *path, const char* buffer, unsigned int size);

	void Draw(unsigned int shader);

	void DrawProperties();

private:
	/*  Model Data  */
	const char* buffer_file = nullptr;
	unsigned int buffer_size = 0;
	bool droped = false;
	std::vector<RE_UnregisteredMesh> meshes;
	std::string directory;
	std::vector<_Texture> textures_loaded;

	bool show_f_normals = true;
	bool show_v_normals = false;

	/*  Functions   */
	void loadModel(std::string path);
	void processNode(aiNode *node, const aiScene *scene);
	RE_UnregisteredMesh processMesh(aiMesh *mesh, const aiScene *scene);
	std::vector<_Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
		std::string typeName);
};
#endif // !__RE_COMPMESH_H__