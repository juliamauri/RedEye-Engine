#ifndef __RE_COMPONENT_H__
#define __RE_COMPONENT_H__

#include "RE_DataTypes.h"
#include "RE_Serializable.h"

#include <EASTL/vector.h>
#include "MGL/Math/float3.h"

class RE_GameObject;
class GameObjectsPool;

class RE_Component : public RE_Serializable
{
public:

	enum class Type : ushort
	{
		EMPTY = 0,

		TRANSFORM = 1,
		MESH = 2,
		CAMERA = 3,
		LIGHT = 4,
		WATER = 5,
		PARTICLEEMITER = 6,

		PRIMIVE_MIN = 20,
		GRID = 21,
		CUBE = 22,
		POINT = 23,
		DODECAHEDRON = 24,
		TETRAHEDRON = 25,
		OCTOHEDRON = 26,
		ICOSAHEDRON = 27,
		PLANE = 28,
		FUSTRUM = 29,
		SPHERE = 30,
		CYLINDER = 31,
		HEMISHPERE = 32,
		TORUS = 33,
		TREFOILKNOT = 34,
		ROCK = 35,
		PRIMIVE_MAX = 36,

		MAX = 37
	};

protected:

	bool active = true;
	Type type = Type::EMPTY;

	COMP_UID id = 0;
	GO_UID go = 0;

	GameObjectsPool* pool_gos = nullptr;
	bool useParent = true;

public:

	RE_Component(const Type type = Type::EMPTY, const GO_UID go = 0, const bool start_active = true) :
		type(type), go(go), active(start_active) {}
	virtual ~RE_Component() = default;

	virtual COMP_UID PoolSetUp(GameObjectsPool* pool, const GO_UID parent, bool report_parent = false);
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
	template <typename T> T As() const { return dynamic_cast<T>(this); }

	GO_UID GetGOUID() const { return go; }
	RE_GameObject* GetGOPtr() const;
	const RE_GameObject* GetGOCPtr() const;
	void SetParent(const GO_UID parent) { useParent = (go = parent); };
	math::vec GetGlobalPosition() const;

	//POOL
	COMP_UID GetPoolID() const { return id; }
	void SetPoolID(COMP_UID uid) { id = uid; }

	// Resources
	virtual void UseResources() {}
	virtual void UnUseResources() {}
	virtual eastl::vector<const char*> GetAllResources() const { return eastl::vector<const char*>(); }

	// Serialization
	virtual void JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources = nullptr) const override;
	virtual void JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources = nullptr) override;
	virtual size_t GetBinarySize() const override { return 0; }
	virtual void BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources = nullptr) const override {}
	virtual void BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources = nullptr) override {}
};

#endif // !__RE_COMPONENT_H__