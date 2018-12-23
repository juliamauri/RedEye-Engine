#ifndef __RE_MESH_H__
#define __RE_MESH_H__

#include "RE_Math.h"
#include "Resource.h"

struct Texture2D;

struct Vertex
{
	math::vec Position = math::vec::zero;
	math::vec Normal = math::vec::zero;
	math::float2 TexCoords = math::float2::zero;
};

struct Texture
{
	std::string id;
	std::string type;
	std::string path;

	Texture2D* ptr = nullptr;
};

class RE_Mesh : public ResourceContainer
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
#endif // !__RE_MESH_H__