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

class RE_UnregisteredMesh
{
public:
	RE_UnregisteredMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, unsigned int triangles);
	//~RE_UnregisteredMesh();

	void Draw(unsigned int shader_ID, bool f_normals = false, bool v_normals = false);

private:

	void _setupMesh();

public:

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	std::string name;
	unsigned int triangle_count = 0;
	unsigned int VAO;
	unsigned int VAO_Vertex, VAO_FaceNormals, VAO_VertexNormals;

	void loadVertexNormals();
	void loadFaceNormals();
	void clearVertexNormals();
	void clearFaceNormals();
	bool lVertexNormals = false, lFaceNormals = false;

private:
	unsigned int VBO, EBO;
	unsigned int VBO_Vertex, VBO_FaceNormals, VBO_VertexNormals;

	void loadVertex();
	void clearVertex();
};

class RE_CompUnregisteredMesh
{
public:
	RE_CompUnregisteredMesh(char *path);
	RE_CompUnregisteredMesh(char *path, const char* buffer, unsigned int size);

	void Draw(unsigned int shader);
	void DrawProperties();

private:

	void loadModel(std::string path);
	void processNode(aiNode *node, const aiScene *scene);
	RE_UnregisteredMesh processMesh(aiMesh *mesh, const aiScene *scene, const unsigned int pos = 1);
	std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);

public:

	const char* buffer_file = nullptr;
	std::string name;
	std::string directory;
	unsigned int buffer_size = 0;

	bool dropped = false;
	unsigned int total_triangle_count = 0;

	std::vector<RE_UnregisteredMesh> meshes;
	std::vector<Texture> textures_loaded;

	math::AABB bounding_box;

	bool error_loading = false;

	bool show_f_normals = false;
	bool show_v_normals = false;
};


#endif // !__RE_MESH_H__