#ifndef __RE_MESH_H__
#define __RE_MESH_H__

#include "RE_Math.h"
#include "Resource.h"

class RE_Mesh : public ResourceContainer
{
public:
	RE_Mesh();
	RE_Mesh(const char* metaPath);
	~RE_Mesh();

	void LoadInMemory() override;
	void UnloadMemory() override;

	const char* CheckAndSave(bool* exists);

	void DrawMesh(const float* transform, unsigned int shader, const char* materialMD5, unsigned int checker,  bool use_checkers = false);

	math::AABB GetAABB() const;

	void loadVertexNormals();
	void loadFaceNormals();
	void clearVertexNormals();
	void clearFaceNormals();

	void SetVerticesAndIndex(float* vertex, unsigned int* index = nullptr, unsigned int triangleCount = 0, float* textureCoords = nullptr, float* normals = nullptr, float* tangents = nullptr, float* bitangents = nullptr);

public:
	bool lVertexNormals = false, lFaceNormals = false;

private:
	void Draw() override;

	void SetupAABB();
	void SetupMesh();

	void LoadVertex();
	void ClearVertex();

	void LibraryLoad();

private:
	float* vertex = nullptr;
	float* normals = nullptr;
	float* tangents = nullptr;
	float* bitangents = nullptr;
	float* texturecoords = nullptr;
	unsigned int* index = nullptr;

	unsigned int triangle_count = 0;

	math::AABB bounding_box;

	unsigned int VAO,VBO, EBO;

	float* vertexNormals = nullptr;
	float* faceNormals = nullptr;
	float* faceCenters = nullptr;
	unsigned int VAO_Vertex, VAO_FaceNormals, VAO_VertexNormals, VAO_FaceCenters;
	unsigned int VBO_Vertex, VBO_FaceNormals, VBO_VertexNormals, VBO_FaceCenters;
};
#endif // !__RE_MESH_H__