#ifndef __RE_COMPMESH_H__
#define __RE_COMPMESH_H__

#include "RE_Component.h"
#include "MathGeoLib/include/Geometry/AABB.h"

class RE_CompMesh : public RE_Component
{
public:
	RE_CompMesh();
	~RE_CompMesh();

	void SetUp(RE_GameObject* parent, const char* reference = nullptr);
	void SetUp(const RE_CompMesh& cmpMesh, RE_GameObject* parent = nullptr);

	void Draw() override;

	void DrawProperties() override;

	unsigned int GetVAOMesh()const;
	unsigned int GetTriangleMesh()const;

	void SetMaterial(const char* md5);
	const char* GetMaterial()const;

	eastl::vector<const char*> GetAllResources() override;
	
	void SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources) override;
	void DeserializeJson(JSONNode* node, eastl::map<int, const char*>* resources, RE_GameObject* parent)override;

	unsigned int GetBinarySize()const override;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) override;
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources, RE_GameObject* parent)override;

	math::AABB GetAABB() const;
	bool CheckFaceCollision(const math::Ray &ray, float &distance) const;

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