#ifndef __RE_COMPONENT_H__
#define __RE_COMPONENT_H__

#include "RE_DataTypes.h"

#include <EASTL/vector.h>
#include <EASTL/map.h>

class RE_Component
{
public:

	enum class Type : ushort
	{
		EMPTY = 0x00,
		TRANSFORM,
		MESH,
		CAMERA,
		LIGHT,
		WATER,
		PARTICLEEMITER,

		PRIMIVE_MIN = 20,
		GRID,
		CUBE,
		POINT,
		DODECAHEDRON,
		TETRAHEDRON,
		OCTOHEDRON,
		ICOSAHEDRON,
		PLANE,
		FUSTRUM,
		SPHERE,
		CYLINDER,
		HEMISHPERE,
		TORUS,
		TREFOILKNOT,
		ROCK,
		PRIMIVE_MAX,

		MAX
	};

	RE_Component(const Type type = Type::EMPTY, const GO_UID go = 0, const bool start_active = true) :
		type(type), go(go), active(start_active) {}
	virtual ~RE_Component() {}

	virtual COMP_UID PoolSetUp(class GameObjectsPool* pool, const GO_UID parent, bool report_parent = false);
	virtual void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent) {}

	virtual void Init() {}
	virtual void CleanUp() {}

	virtual void PreUpdate() {}
	virtual void Update() {}
	virtual void PostUpdate() {}

	virtual void Draw() const {}
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

	void SetType(Type t) { type = t; }
	Type GetType() const { return type; }
	GO_UID GetGOUID() const { return go; }
	class RE_GameObject* GetGOPtr() const;
	const RE_GameObject* GetGOCPtr() const;
	void SetParent(const GO_UID parent) { useParent = (go = parent); };

	virtual eastl::vector<const char*> GetAllResources() { return eastl::vector<const char*>(); }

	virtual void SerializeJson(class RE_Json* node, eastl::map<const char*, int>* resources) const {}
	virtual void DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources) {}

	virtual unsigned int GetBinarySize() const { return 0; }
	virtual void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const {}
	virtual void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources) {}

	virtual void UseResources() {}
	virtual void UnUseResources() {}

	//POOL
	COMP_UID GetPoolID()const { return id; }
	void SetPoolID(COMP_UID uid) { id = uid; }

protected:

	bool active = true;
	Type type = Type::EMPTY;

	COMP_UID id = 0;
	GO_UID go = 0;

	GameObjectsPool* pool_gos = nullptr;
	bool useParent = true;
};

#endif // !__RE_COMPONENT_H__