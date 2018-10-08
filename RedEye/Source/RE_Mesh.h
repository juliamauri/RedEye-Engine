#ifndef __RE_MESH_H__
#define __RE_MESH_H__

#include "RE_Math.h"

struct Vertex
{
	math::vec Position;
	math::vec Normal;
	math::float2 TexCoords;
};

struct Texture
{
	unsigned int id;
	std::string type;
	std::string path;
};

class RE_Mesh
{
public:
	RE_Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	~RE_Mesh();

	void Draw(unsigned int shader_ID);

private:

	void setupMesh();

private:

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	unsigned int VAO, VBO, EBO;
	bool useIndex = true;
};

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;

class RE_MeshContainer
{
public:

	RE_MeshContainer(const char* file = nullptr, const char* directory = nullptr, bool dropped = false);
	~RE_MeshContainer();

	void Draw(unsigned int shader);

	void	ProcessNode(aiNode* node, const aiScene* scene);
	RE_Mesh	ProcessMesh(aiMesh* mesh, const aiScene* scene);

	std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

	const char* GetFileName() const;

private:

	std::string file_name;
	std::string directory;
	bool dropped = false;

	bool show_face_normals = true;
	bool show_vertex_normals = false;

	std::vector<RE_Mesh> meshes;
	std::vector<Texture> textures_loaded;
};

#endif // !__RE_MESH_H__