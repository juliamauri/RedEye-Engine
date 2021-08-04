#ifndef __RE_COMPMESH_H__
#define __RE_COMPMESH_H__

#include "RE_Component.h"
#include "MathGeoLib/include/Geometry/AABB.h"

class RE_CompMesh : public RE_Component
{
public:
	RE_CompMesh() : RE_Component(C_MESH) {}
	~RE_CompMesh() {}
	
	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent) override;

	void Draw() const override;
	void DrawProperties() override;

	void SetMesh(const char* mesh);
	const char* GetMesh() const;

	unsigned int GetVAOMesh()const;
	unsigned int GetTriangleMesh()const;

	void SetMaterial(const char* md5);
	const char* GetMaterial()const;

	eastl::vector<const char*> GetAllResources() override;
	
	void SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources) const override;
	void DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources) override;

	unsigned int GetBinarySize()const override;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const override;
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources) override;

	math::AABB GetAABB() const;
	bool CheckFaceCollision(const math::Ray &local_ray, float &distance) const;

	void UseResources()override;
	void UnUseResources()override;

	bool isBlend()const;

protected:

	const char* meshMD5 = nullptr;
	const char* materialMD5 = nullptr;

	bool show_checkers = false;
	bool show_f_normals = false;
	bool show_v_normals = false;
};

#endif // !__RE_COMPMESH_H__