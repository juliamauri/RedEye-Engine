#ifndef __RE_COMPONENT_H__
#define __RE_COMPONENT_H__

#include "Globals.h"

#include <vector>
#include <map>

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
	C_GRID,
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
	RE_Component(const ComponentType type = C_EMPTY, RE_GameObject* go = nullptr, const bool start_active = true) : 
		type(type), go(go), active(start_active) {}
	virtual ~RE_Component() {}

	virtual void Init() {}
	virtual void CleanUp() {}

	virtual void PreUpdate() {}
	virtual void Update() {}
	virtual void PostUpdate() {}

	virtual void Draw() {}
	virtual void DrawProperties() {}

	virtual void OnPlay() {}
	virtual void OnPause() {}
	virtual void OnStop() {}

	virtual void Save() const {}
	virtual void Load() {}
	virtual void Reset() {}

	virtual void OnTransformModified() {}

	bool IsActive() const { return active; }
	void SetActive(const bool value) { active = value; }

	ComponentType GetType() const { return type; }
	RE_GameObject* GetGO() const { return go; }

	RE_Component* AsComponent() const { return (RE_Component*)this; }

	virtual std::vector<const char*> GetAllResources() { return std::vector<const char*>(); }

	virtual unsigned int GetBinarySize()const {  return 0; }
	virtual void SerializeJson(JSONNode* node, std::map<const char*, int>* resources) {}
	virtual void SerializeBinary(char*& cursor, std::map<const char*, int>* resources) {}

	virtual void UseResources() {  }
	virtual void UnUseResources() {  }

protected:

	bool active = true;
	ComponentType type = C_EMPTY;
	RE_GameObject* go = nullptr;
};

#endif // !__RE_COMPONENT_H__