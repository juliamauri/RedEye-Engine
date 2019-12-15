#ifndef __RE_MESH_H__
#define __RE_MESH_H__

#include "RE_Math.h"
#include "Resource.h"

class RE_Mesh : public ResourceContainer
{
public:
	RE_Mesh();
	~RE_Mesh();

	void SetLibraryPath(const char* path)override;

	void LoadInMemory() override;
	void UnloadMemory() override;

	const char* CheckAndSave(bool* exists);

	void DrawMesh(unsigned int shader);

	math::AABB GetAABB() const;

	void loadVertexNormals();
	void loadFaceNormals();
	void clearVertexNormals();
	void clearFaceNormals();

	void SetVerticesAndIndex(float* vertex, unsigned int* index = nullptr, unsigned int vertexCount = 0, unsigned int triangleCount = 0, float* textureCoords = nullptr, float* normals = nullptr, float* tangents = nullptr, float* bitangents = nullptr);

	bool CheckFaceCollision(const math::Ray& local_ray, float& distance) const;

	unsigned int GetVAO()const { return VAO; }
	unsigned int GetTriangleCount()const { return triangle_count; }

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
	unsigned int vertex_count = 0;

	math::AABB bounding_box;

	unsigned int VAO,VBO, EBO;

	float* vertexNormals = nullptr;
	float* faceNormals = nullptr;
	float* faceCenters = nullptr;
	unsigned int VAO_Vertex, VAO_FaceNormals, VAO_VertexNormals, VAO_FaceCenters;
	unsigned int VBO_Vertex, VBO_FaceNormals, VBO_VertexNormals, VBO_FaceCenters;
};
#endif // !__RE_MESH_H__