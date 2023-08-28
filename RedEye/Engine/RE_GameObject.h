#ifndef __RE_GAMEOBJECT_H__
#define __RE_GAMEOBJECT_H__

#include "RE_Component.h"

#include <MGL/Geometry/AABB.h>
#include <EASTL/list.h>
#include <EASTL/stack.h>
#include <EASTL/string.h>

class RE_GameObject
{
public:

	RE_GameObject();
	~RE_GameObject();

	friend class GameObjectsPool;
	friend class RE_ECS_Pool;

	void SetUp(GameObjectsPool* goPool, class ComponentsPool* compPool, const char* name,
		const GO_UID parent = 0, const bool start_active = true, const bool isStatic = true);

	// Draws
	void DrawProperties();
	void DrawItselfOnly() const;
	void DrawWithChilds() const;
	void DrawChilds() const;

	// Component Getters
	class RE_CompTransform* GetTransformPtr() const;
	class RE_Component* GetRenderGeo() const;
	class RE_CompMesh* GetMesh() const;
	class RE_CompWater* GetWater() const;
	class RE_CompParticleEmitter* GetParticleSystem() const;
	class RE_CompCamera* GetCamera() const;
	class RE_CompLight* GetLight() const;
	class RE_CompPrimitive* GetPrimitive() const;

	RE_Component* GetCompPtr(const RE_Component::Type type) const;
	COMP_UID GetCompUID(const RE_Component::Type type) const;
	bool HasRenderGeo() const;
	bool HasActiveRenderGeo() const;

	eastl::list<RE_Component*> GetComponentsPtr() const;
	eastl::list<RE_Component*> GetStackableComponentsPtr() const;
	eastl::stack<RE_Component*> GetAllChildsComponents(const RE_Component::Type type) const;
	eastl::stack<RE_Component*> GetAllChildsActiveRenderGeos() const;
	eastl::stack<RE_Component*> GetAllChildsActiveRenderGeos(const GO_UID stencil_mask) const;

	// Components
	void ReportComponent(const COMP_UID id, const RE_Component::Type type);
	RE_Component* AddNewComponent(const RE_Component::Type type);
	void ReleaseComponent(const COMP_UID id, const RE_Component::Type type);
	void DestroyComponent(const COMP_UID id, const RE_Component::Type type);

	// Children Getters
	const eastl::vector<GO_UID>& GetChilds() const;
	eastl::list<RE_GameObject*> GetChildsPtr() const;
	eastl::list<const RE_GameObject*> GetChildsCPtr() const;
	eastl::list<RE_GameObject*> GetChildsPtrReversed() const;

	eastl::vector<GO_UID> GetGOandChilds() const;
	eastl::vector<RE_GameObject*> GetGOandChildsPtr();
	eastl::vector<const RE_GameObject*> GetGOandChildsCPtr() const;

	eastl::vector<RE_GameObject*> GetActiveDrawableChilds() const;
	eastl::vector<RE_GameObject*> GetActiveDrawableGOandChildsPtr();
	eastl::vector<const RE_GameObject*> GetActiveDrawableGOandChildsCPtr() const;

	const GO_UID GetFirstChildUID() const;
	RE_GameObject* GetFirstChildPtr() const;
	const RE_GameObject* GetFirstChildCPtr() const;

	const GO_UID GetLastChildUID() const;
	RE_GameObject* GetLastChildPtr() const;
	const RE_GameObject* GetLastChildCPtr() const;

	// Children
	RE_GameObject* AddNewChild(const char* name = nullptr, const bool start_active = true, const bool isStatic = true);
	void ReleaseChild(const GO_UID id);
	void DestroyChild(const GO_UID id);

	eastl_size_t ChildCount() const;
	bool IsLastChild() const;

	// Parent
	bool isParent(GO_UID parent)const;
	GO_UID GetParentUID() const;
	RE_GameObject* GetParentPtr() const;
	const RE_GameObject* GetParentCPtr() const;
	void SetParent(const GO_UID id, bool unlink_previous = true, bool link_new = true);
	void UnlinkParent();

	GO_UID GetRootUID() const;
	RE_GameObject* GetRootPtr() const;
	const RE_GameObject* GetRootCPtr() const;

	// Flags
	bool IsActive() const;
	bool IsStatic() const;
	bool IsActiveStatic() const;
	bool IsActiveNonStatic() const;

	void SetActive(const bool value, bool broadcast = true);
	void SetActiveWithChilds(bool val, bool broadcast = true);
	
	void SetStatic(const bool value, bool broadcast = true);
	void SetStaticWithChilds(bool val, bool broadcast = true);

	// Events
	void OnPlay();
	void OnPause();
	void OnStop();
	void OnTransformModified();

	// AABB
	inline void AddToBoundingBox(math::AABB box);
	void ResetLocalBoundingBox();
	void ResetGlobalBoundingBox();
	void ResetBoundingBoxes();
	void ResetGOandChildsAABB();
	void DrawAABB(math::vec color) const;
	void DrawGlobalAABB() const;

	// AABB Getters
	math::AABB GetLocalBoundingBox() const;
	math::AABB GetGlobalBoundingBox() const;
	math::AABB GetGlobalBoundingBoxWithChilds() const;

	// Raycast
	bool CheckRayCollision(const math::Ray& global_ray, float& distance) const;

	//POOL
	GO_UID GetUID() const;
	struct ComponentData { COMP_UID uid = 0ull; RE_Component::Type type = RE_Component::Type(0); };

	// Resources
	void UseResources();
	void UnUseResources();

	// Serialization
	size_t GetBinarySize() const;
	void SerializeJson(class RE_Json* node);
	void DeserializeJSON(RE_Json* node, GameObjectsPool* goPool, ComponentsPool* cmpsPool);
	void SerializeBinary(char*& cursor);
	void DeserializeBinary(char*& cursor, GameObjectsPool* goPool, ComponentsPool* compPool);

private:

	inline RE_Component* CompPtr(ComponentData comp) const;
	inline RE_Component* CompPtr(COMP_UID id, RE_Component::Type type) const;
	inline const RE_Component* CompCPtr(ComponentData comp) const;
	inline const RE_Component* CompCPtr(COMP_UID id, RE_Component::Type type) const;
	eastl::list<ComponentData> AllCompData() const;

	inline bool IsRenderGeo(RE_Component::Type type) const;

	inline RE_GameObject* ChildPtr(const GO_UID child) const;
	inline const RE_GameObject* ChildCPtr(const GO_UID child) const;

public:
	
	eastl::string name;

private:


	/* Todo Rub: bitmask go flags with properties
	enum GO_Flags : char { ACTIVE, PARENT_ACTIVE, STATIC, DYNAMIC, KINEMATIC, HAS_CHILDS, IS_ROOT };*/

	enum class Flag : ushort
	{
		ACTIVE = 0x1,			 // 0000 0000 0000 0001
		PARENT_ACTIVE = 0x2,	 // 0000 0000 0000 0010
		HAS_CHILDS = 0x4,		 // 0000 0000 0000 0100
		IS_ROOT = 0x8,			 // 0000 0000 0000 1000

		STATIC = 0x10,			 // 0000 0000 0001 0000
		DYNAMIC = 0x20,			 // 0000 0000 0010 0000
		KINEMATIC = 0x40,		 // 0000 0000 0100 0000
	};

	bool active = true;
	bool isStatic = true;

	GO_UID go_uid = 0ull;
	GO_UID parent_uid = 0ull;

	ComponentData render_geo; // mesh or primitive
	COMP_UID transform = 0ull;
	COMP_UID camera = 0ull;
	COMP_UID light = 0ull;

	math::AABB local_bounding_box;
	math::AABB global_bounding_box;

	eastl::vector<GO_UID> childs;
	eastl::vector<ComponentData> components;

	ComponentsPool* pool_comps = nullptr;
	GameObjectsPool* pool_gos = nullptr;
};

#endif // !__RE_GAMEOBJECT_H__