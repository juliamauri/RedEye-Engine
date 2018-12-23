#ifndef __RE_COMPONENT_H__
#define __RE_COMPONENT_H__

#include "Globals.h"

#include "RapidJson\include\rapidjson.h"
#include "RapidJson\include\document.h"
#include "RapidJson\include\allocators.h"

class RE_GameObject;
class JSONNode;

enum ComponentType : ushortint
{
	C_EMPTY = 0x00,
	C_TRANSFORM,
	C_PRIMITIVE,
	C_AXIS,
	C_POINT,
	C_LINE,
	C_RAY,
	C_TRIANGLE,
	C_PLANE,
	C_CUBE,
	C_FUSTRUM,
	C_SPHERE,
	C_CYLINDER,
	C_CAPSULE,
	C_MESH,
	C_CAMERA,
	C_PARTICLEEMITER,
	MAX_COMPONENT_TYPES
};

class RE_Component
{
public:
	RE_Component(const ComponentType type = C_EMPTY, RE_GameObject* go = nullptr, const bool start_active = true);
	virtual ~RE_Component() {}

	virtual void Init() {}
	virtual void CleanUp() {}

	virtual void PreUpdate() {}
	virtual void Update() {}
	virtual void PostUpdate() {}

	virtual void Draw() {}
	virtual void DrawProperties() {}

	virtual void Save() const {}
	virtual void Load() {}
	virtual void Reset() {}

	virtual void OnTransformModified() {}

	bool IsActive() const;
	void SetActive(const bool value);

	ComponentType GetType() const;
	RE_GameObject* GetGO() const;

	RE_Component* AsComponent() const;

	virtual void Serialize(JSONNode* node, rapidjson::Value* val);

protected:

	bool active = true;
	ComponentType type = C_EMPTY;
	RE_GameObject* go = nullptr;
};

#endif // !__RE_COMPONENT_H__