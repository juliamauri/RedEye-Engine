#ifndef __RE_MESH_H__
#define __RE_MESH_H__

class RE_Mesh : public ResourceContainer
{
public:
	RE_Mesh();
	~RE_Mesh() = default;

	void SetLibraryPath(const char* path) override;

	void LoadInMemory() override;
	void UnloadMemory() override;

	const char* CheckAndSave(bool* exists);

	void DrawMesh(uint shader);

	math::AABB GetAABB() const;

	void loadVertexNormals();
	void loadFaceNormals();
	void clearVertexNormals();
	void clearFaceNormals();

	void SetVerticesAndIndex(
		float* vertex,
		uint* index = nullptr,
		size_t vertexCount = 0,
		uint triangleCount = 0,
		float* textureCoords = nullptr,
		float* normals = nullptr,
		float* tangents = nullptr,
		float* bitangents = nullptr);

	bool CheckFaceCollision(const math::Ray& local_ray, float& distance) const;

	uint GetVAO() const { return VAO; }
	size_t GetTriangleCount() const { return triangle_count; }

public:

	bool lVertexNormals = false;
	bool lFaceNormals = false;

private:

	void Draw() override;

	void SetupAABB();
	void SetupMesh();

	void LoadVertex();
	void ClearVertex();

	void LibraryLoad();

private:

	uint* index = nullptr;

	float* vertex = nullptr;
	float* normals = nullptr;
	float* tangents = nullptr;
	float* bitangents = nullptr;
	float* texturecoords = nullptr;

	size_t triangle_count = 0;
	size_t vertex_count = 0;

	math::AABB bounding_box;

	uint VAO = 0u;
	uint VBO = 0u;
	uint EBO = 0u;

	float* vertexNormals = nullptr;
	float* faceNormals = nullptr;
	float* faceCenters = nullptr;

	uint VAO_Vertex = 0u;
	uint VAO_FaceNormals = 0u;
	uint VAO_VertexNormals = 0u;
	uint VAO_FaceCenters = 0u;

	uint VBO_Vertex = 0u;
	uint VBO_FaceNormals = 0u;
	uint VBO_VertexNormals = 0u;
	uint VBO_FaceCenters = 0u;
};
#endif // !__RE_MESH_H__