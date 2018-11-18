#ifndef __RE_MESH_H__
#define __RE_MESH_H__

#include "RE_Math.h"
#include "Resource.h"

struct Vertex
{
	math::vec Position;
	math::vec Normal;
	math::float2 TexCoords;
};

struct Texture
{
	std::string id;
	std::string type;
	std::string path;
};

class RE_Mesh : ResourceContainer
{
public:
	RE_Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, unsigned int triangles);
	~RE_Mesh();

	void Draw(unsigned int shader_ID);

	std::string name;
	unsigned int triangle_count = 0;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	math::AABB GetAABB();

	void loadVertexNormals();
	void loadFaceNormals();
	void clearVertexNormals();
	void clearFaceNormals();
	bool lVertexNormals = false, lFaceNormals = false;
	unsigned int VAO;

private:

	void SetupAABB();
	void setupMesh();

private:

	math::AABB bounding_box;
	unsigned int VAO_Vertex, VAO_FaceNormals, VAO_VertexNormals;

private:
	unsigned int VBO, EBO;
	unsigned int VBO_Vertex, VBO_FaceNormals, VBO_VertexNormals;

	void loadVertex();
	void clearVertex();
};

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;

class RE_MeshContainer
{
public:

	RE_MeshContainer(const aiScene* scene, const char* file = nullptr, const char* directory = nullptr, bool dropped = false);
	~RE_MeshContainer();

	void Draw(unsigned int shader);

	void	ProcessNode(aiNode* node, const aiScene* scene);
	RE_Mesh	ProcessMesh(aiMesh* mesh, const aiScene* scene, const unsigned int pos);

	std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

	void DrawProperties();

	const char* GetFileName() const;

	unsigned int total_triangle_count = 0;
	bool error_loading = false;

private:

	std::string file_name;
	std::string directory;
	bool dropped = false;
	math::AABB bounding_box;

	bool show_face_normals = true;
	bool show_vertex_normals = false;

	std::vector<RE_Mesh> meshes;
	std::vector<Texture> textures_loaded;

	bool show_f_normals = false;
	bool show_v_normals = false;
};
#endif // !__RE_MESH_H__