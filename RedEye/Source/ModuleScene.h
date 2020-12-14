#ifndef __MODULESCENE_H__
#define __MODULESCENE_H__

#include "Module.h"
#include "RE_ECS_Pool.h"
#include "RE_AABBDynTree.h"
#include <EASTL/stack.h>

class RE_Scene;
class RE_CameraManager;
class RE_PrimitiveManager;

class ModuleScene : public Module
{
public:
	ModuleScene();
	~ModuleScene();

	bool Init() override;
	bool Start() override;
	void Update() override;
	void PostUpdate() override;
	void CleanUp() override;
	void DrawEditor() override;
	void RecieveEvent(const Event& e) override;

	// Draw space Partitioning
	void DebugDraw() const;
	
	// Events
	void OnPlay();
	void OnPause();
	void OnStop();

	// Current Pool
	RE_ECS_Pool* GetScenePool() { return &scenePool; }
	RE_GameObject* GetGOPtr(UID id) const { return scenePool.GetGOPtr(id); }
	const RE_GameObject* GetGOCPtr(UID id) const { return scenePool.GetGOCPtr(id); }

	// Root
	UID GetRootUID() const { return scenePool.GetRootUID(); }
	RE_GameObject* GetRootPtr() const { return scenePool.GetRootPtr(); }
	const RE_GameObject* GetRootCPtr() const { return scenePool.GetRootCPtr(); }

	// Adding to scene
	void CreatePrimitive(ComponentType type, const UID parent = 0);
	void CreateCamera(const UID parent = 0);
	void CreateLight(const UID parent = 0);
	void CreateMaxLights(const UID parent = 0);
	void CreateWater(const UID parent = 0);
	void AddGOPool(RE_ECS_Pool* toAdd);

	// Scene Gameobject Filtering
	UID RayCastGeometry(math::Ray& global_ray) const;
	void FustrumCulling(eastl::vector<const RE_GameObject*>& container, const math::Frustum& frustum) const;

	// Scene Management
	const char* GetCurrentScene() const;
	void ClearScene();
	void NewEmptyScene(const char* name = "New Scene");
	bool HasChanges() const { return haschanges; }
	bool isNewScene() const { return (unsavedScene); }

	// Serialization
	void LoadScene(const char* sceneMD5, bool ignorehandle = false);
	void SaveScene(const char* newName = nullptr);
	
private:

	void SetupScene();

	UID Validate(const UID id) const { return id ? id : GetRootUID(); }

public:

	RE_CameraManager* cams = nullptr;
	RE_PrimitiveManager* primitives = nullptr;

private:

	RE_ECS_Pool scenePool;
	RE_ECS_Pool savedState;
	bool haschanges = false;

	// Trees
	RE_AABBDynTree static_tree;
	RE_AABBDynTree dynamic_tree;

	eastl::stack<UID> to_delete;

	RE_Scene* unsavedScene = nullptr;
	const char* currentScene = nullptr;
};

#endif // !__MODULESCENE_H__