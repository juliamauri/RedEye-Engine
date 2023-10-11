#ifndef __RE_COMPMESH_H__
#define __RE_COMPMESH_H__

class RE_CompMesh : public RE_Component
{
public:
	RE_CompMesh() : RE_Component(RE_Component::Type::MESH) {}
	~RE_CompMesh() final = default;
	
	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent) final;

	void Draw() const final;
	void DrawProperties() final;

	void SetMesh(const char* mesh);
	const char* GetMesh() const;

	unsigned int GetVAOMesh() const;
	size_t GetTriangleMesh() const;

	void SetMaterial(const char* md5);
	const char* GetMaterial() const;

	eastl::vector<const char*> GetAllResources() final;
	
	void SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources) const final;
	void DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources) final;

	size_t GetBinarySize() const override;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const final;
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources) final;

	math::AABB GetAABB() const;
	bool CheckFaceCollision(const math::Ray &local_ray, float &distance) const;

	void UseResources() final;
	void UnUseResources() final;

	bool HasBlend()const;

protected:

	const char* meshMD5 = nullptr;
	const char* materialMD5 = nullptr;

	bool show_checkers = false;
	bool show_f_normals = false;
	bool show_v_normals = false;
};

#endif // !__RE_COMPMESH_H__