#ifndef __RE_COMPMESH_H__
#define __RE_COMPMESH_H__

#include "RE_Component.h"
#include <string>

class RE_CompMesh : public RE_Component
{
public:
	RE_CompMesh(RE_GameObject* go = nullptr, const char* reference = nullptr, const bool start_active = true);
	~RE_CompMesh();

	void Draw() override;

	void DrawProperties() override;

	void Serialize(JSONNode* node, rapidjson::Value* val) override;

protected:

	std::string reference;
};


#endif // !__RE_COMPMESH_H__