#ifndef __RE_COMPMESH_H__
#define __RE_COMPMESH_H__

#include "RE_Component.h"
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

	void Serialize(JSONNode* node, rapidjson::Value* val) override;

	void SetTexture(const char* reference, const char* file_path, const char* type);


protected:

	std::string reference;
	RE_Mesh* ptr = nullptr;
};


#endif // !__RE_COMPMESH_H__