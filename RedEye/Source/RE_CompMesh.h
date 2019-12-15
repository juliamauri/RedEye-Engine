#ifndef __RE_COMPMESH_H__
#define __RE_COMPMESH_H__

#include "RE_Component.h"
#include "MathGeoLib/include/Geometry/AABB.h"
#include <string>

class RE_CompMesh : public RE_Component
{
public:
	RE_CompMesh(RE_GameObject* go = nullptr, const char* reference = nullptr, const bool start_active = true);
	RE_CompMesh(const RE_CompMesh& cmpMesh, RE_GameObject* go = nullptr);
	~RE_CompMesh();

	void Draw() override;

	void DrawProperties() override;

	unsigned int GetVAOMesh()const;
	unsigned int GetTriangleMesh()const;

	void SetMaterial(const char* md5);
	const char* GetMaterial()const;

	std::vector<const char*> GetAllResources() override;

	unsigned int GetBinarySize()const override;
	void SerializeJson(JSONNode* node, std::map<const char*, int>* resources) override;
	void SerializeBinary(char*& cursor, std::map<const char*, int>* resources) override;

	math::AABB GetAABB() const;
	bool CheckFaceCollision(const math::Ray &ray, float &distance) const;

	void UseResources();
	void UnUseResources();

protected:

	const char* meshMD5 = nullptr;
	const char* materialMD5 = nullptr;

	bool show_checkers = false;
	bool show_f_normals = false;
	bool show_v_normals = false;
};


#endif // !__RE_COMPMESH_H__