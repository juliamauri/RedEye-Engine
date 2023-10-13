#ifndef __RE_COMPMESH_H__
#define __RE_COMPMESH_H__

class RE_CompMesh : public RE_Component
{
protected:

	const char* meshMD5 = nullptr;
	const char* materialMD5 = nullptr;

	bool show_checkers = false;
	bool show_f_normals = false;
	bool show_v_normals = false;

public:
	RE_CompMesh() : RE_Component(RE_Component::Type::MESH) {}
	~RE_CompMesh() final = default;
	
	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent) final;

	void Draw() const final;
	void DrawProperties() final;

	void SetMesh(const char* mesh);
	const char* GetMesh() const { return meshMD5; }

	unsigned int GetVAOMesh() const;
	size_t GetTriangleMesh() const;

	void SetMaterial(const char* md5) { materialMD5 = md5; }
	const char* GetMaterial() const { return materialMD5; }

	bool HasBlend()const;

	math::AABB GetAABB() const;
	bool CheckFaceCollision(const math::Ray& local_ray, float& distance) const;

	// Resources
	void UseResources() final;
	void UnUseResources() final;
	eastl::vector<const char*> GetAllResources() final;

	// Serialization
	void JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources = nullptr) const final;
	void JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources = nullptr) final;

	size_t GetBinarySize() const final;
	void BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources = nullptr) const final;
	void BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources = nullptr) final;
};

#endif // !__RE_COMPMESH_H__