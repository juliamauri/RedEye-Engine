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

class RE_Mesh : public ResourceContainer
{
public:
	RE_Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, const char* materialMD5, unsigned int triangles);
	~RE_Mesh();

	void Draw(unsigned int shader_ID);

	math::AABB GetAABB() const;

	void loadVertexNormals();
	void loadFaceNormals();
	void clearVertexNormals();
	void clearFaceNormals();

public:

	std::string name;
	unsigned int VAO;
	unsigned int triangle_count = 0;
	const char* materialMD5 = nullptr;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	bool lVertexNormals = false, lFaceNormals = false;

private:

	void SetupAABB();
	void SetupMesh();

	void LoadVertex();
	void ClearVertex();

private:

	math::AABB bounding_box;

	unsigned int VBO, EBO;
	unsigned int VAO_Vertex, VAO_FaceNormals, VAO_VertexNormals, VAO_FaceCenters;
	unsigned int VBO_Vertex, VBO_FaceNormals, VBO_VertexNormals, VBO_FaceCenters;
};
#endif // !__RE_MESH_H__