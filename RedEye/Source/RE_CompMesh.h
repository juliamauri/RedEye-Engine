#ifndef __RE_COMPMESH_H__
#define __RE_COMPMESH_H__

#include "RE_Component.h"
#include "MathGeoLib/include/Geometry/AABB.h"
#include <string>

class RE_Mesh;

class RE_CompMesh : public RE_Component
{
public:
	RE_CompMesh(RE_GameObject* go = nullptr, const char* reference = nullptr, const bool start_active = true);
	RE_CompMesh(const RE_CompMesh& cmpMesh, RE_GameObject* go = nullptr);
	~RE_CompMesh();

	void Draw() override;

	void DrawProperties() override;

	void SetMaterial(const char* md5);

	void Serialize(JSONNode* node, rapidjson::Value* val) override;

	math::AABB GetAABB() const;

protected:

	std::string reference;
	RE_Mesh* ptr = nullptr;
	bool show_f_normals = false;
	bool show_v_normals = false;
};


#endif // !__RE_COMPMESH_H__