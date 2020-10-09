#ifndef __RE_GAMEOBJECT_H__
#define __RE_GAMEOBJECT_H__

#include "Globals.h"
#include "EventListener.h"
#include "MathGeoLib\include\MathGeoLib.h"
#include <EASTL/list.h>
#include <EASTL/vector.h>
#include <EASTL/map.h>
#include <EASTL/stack.h>

class RE_Component;
class RE_CompTransform;
class RE_CompMesh;
class RE_CompCamera;
class RE_CompPrimitive;
class JSONNode;
class ComponentsPool;
class RE_GOManager;

class RE_GameObject : public EventListener
{
public:
	RE_GameObject();
	~RE_GameObject();

	void SetUp(ComponentsPool* compPool, const char* name, RE_GameObject* parent = nullptr, bool start_active = true, bool isStatic = true);

	void PreUpdate();
	void Update();
	void PostUpdate();
	void DrawWithChilds() const;
	void DrawItselfOnly() const;

	eastl::stack<RE_Component*> GetDrawableComponentsWithChilds(RE_GameObject* ignoreStencil = nullptr)const;
	eastl::stack<RE_Component*> GetDrawableComponentsItselfOnly()const;
	eastl::stack<RE_Component*> GetAllComponentWithChilds(unsigned short type)const;
	eastl::list<RE_Component*> GetComponents()const;

	eastl::vector<RE_GameObject*> GetAllGO();
	eastl::vector<RE_GameObject*> GetActiveChildsWithDrawComponents();

	bool HasDrawComponents() const;

	void SerializeJson(JSONNode* node);
	void DeserializeJSON(JSONNode* node, ComponentsPool* cmpsPool, eastl::map<int, RE_GameObject*>* uuidGO);

	unsigned int GetBinarySize()const;
	void SerializeBinary(char*& cursor);
	void DeserializeBinary(char*& cursor, ComponentsPool* compPool, eastl::map<int, RE_GameObject*>* uuidGO);

	// Children
	void AddChild(RE_GameObject* child, bool broadcast = true);
	void AddChildsFromGO(RE_GameObject* go, bool broadcast = true);
	void RemoveChild(RE_GameObject* child, bool broadcast = true); //Breaks the link with the parent but does not delete the child.
	void RemoveAllChilds();
	eastl::list<RE_GameObject*>& GetChilds();
	const eastl::list<RE_GameObject*>& GetChilds() const;
	void GetChilds(eastl::list<const RE_GameObject*>& out_childs) const;
	unsigned int ChildCount() const;
	bool IsLastChild() const;

	// Parent
	RE_GameObject* GetParent() const;
	const RE_GameObject* GetParent_c() const;
	void SetParent(RE_GameObject* parent);

	// Active
	bool IsActive() const;
	void SetActive(const bool value, const bool broadcast = true);
	void SetActiveRecursive(const bool value);
	void IterativeSetActive(bool val);

	// Static
	bool IsStatic() const;
	bool IsActiveStatic() const;
	void SetStatic(const bool value, const bool broadcast = true);
	void IterativeSetStatic(bool val);

	// Editor Controls
	void OnPlay();
	void OnPause();
	void OnStop();

	// Components
	void AddComponent(RE_Component* component);
	RE_Component* AddComponent(const ushortint type);
	RE_Component* GetComponent(const ushortint type) const;
	void RemoveComponent(RE_Component* component);

	RE_CompTransform* AddCompTransform();
	RE_CompMesh* AddCompMesh();
	RE_CompCamera* AddCompCamera(bool toPerspective = true, float near_plane = 1.0f, float far_plane = 5000.0f, float v_fov = 0.523599f, short aspect_ratio_t = 0, bool draw_frustum = true, bool usingSkybox = true, const char* skyboxMD5 = nullptr);
	RE_CompPrimitive* AddCompPrimitive(ushortint type, int _slices = 3, int _stacks = 3, float _radius = 0.1);
	RE_CompPrimitive* AddCompRock(int _rockSeed = 5, int _rockNSubdivvisions = 20);

	RE_CompTransform* GetTransform() const;
	RE_CompMesh* GetMesh() const;
	RE_CompCamera* GetCamera() const;

	void RecieveEvent(const Event& e) override;

	// Transform
	void TransformModified(bool broadcast = true);

	// Name
	const char* GetName() const;

	// AABB
	inline void AddToBoundingBox(math::AABB box);
	void ResetLocalBoundingBox();
	void ResetGlobalBoundingBox();
	void ResetBoundingBoxes();
	void ResetBoundingBoxForAllChilds();
	void ResetGlobalBoundingBoxForAllChilds();
	void DrawAABB(math::vec color) const;
	void DrawGlobalAABB() const;

	math::AABB GetLocalBoundingBox() const;
	math::AABB GetGlobalBoundingBox() const;
	math::AABB GetGlobalBoundingBoxWithChilds() const;

	// Editor
	void DrawProperties();

	//POOL
	int GetPoolID()const;
	void SetPoolID(int id);
	
public:
	struct cmpPoolID {
		int poolId;
		ushortint cType;
		cmpPoolID(int id, ushortint type) : poolId(id), cType(type) { }
	};

private:

	bool active = true;
	bool isStatic = true;

	eastl::string name;
	math::AABB local_bounding_box;
	math::AABB global_bounding_box;

	RE_GameObject* parent = nullptr;
	RE_CompTransform* transform = nullptr;

	eastl::list<RE_GameObject*> childs;
	ComponentsPool* poolComponents;
	eastl::vector<cmpPoolID> componentsID;

	int poolID = 0;
};

#endif // !__RE_GAMEOBJECT_H__