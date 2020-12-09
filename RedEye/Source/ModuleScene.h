#ifndef __MODULESCENE_H__
#define __MODULESCENE_H__

#include "Event.h"
#include "RE_ECS_Pool.h"
#include "Module.h"
#include "RE_AABBDynTree.h"

class RE_GameObject;
class RE_Scene;

class ModuleScene : public Module
{
public:
	ModuleScene(const char* name = "Scene", bool start_enabled = true);
	~ModuleScene();

	bool Start() override;
	void Update() override;
	void PostUpdate() override;
	void CleanUp() override;

	// Draws
	void DrawEditor() override;
	void DebugDraw() const;
	
	// Events
	void OnPlay() override;
	void OnPause() override;
	void OnStop() override;
	void RecieveEvent(const Event& e) override;

	// Current Pool
	static RE_ECS_Pool* GetScenePool();
	static RE_GameObject* GetGOPtr(UID id);
	static const RE_GameObject* GetGOCPtr(UID id);

	// Root
	static UID GetRootUID();
	static RE_GameObject* GetRootPtr();
	static const RE_GameObject* GetRootCPtr();

	// Adding to scene
	static void CreatePrimitive(ComponentType type, const UID parent = 0);
	static void CreateCamera(const UID parent = 0);
	static void CreateLight(const UID parent = 0);
	static void CreateMaxLights(const UID parent = 0);
	static void CreateWater(const UID parent = 0);

	void AddGOPool(RE_ECS_Pool* toAdd);

	// Scene Gameobject Filtering
	UID RayCastGeometry(math::Ray& global_ray);
	void FustrumCulling(eastl::vector<const RE_GameObject*>& container, const math::Frustum& frustum);

	// Scene Management
	const char* GetCurrentScene()const;
	void ClearScene();
	void NewEmptyScene(const char* name = "New Scene");

	// Serialization
	void LoadScene(const char* sceneMD5, bool ignorehandle = false);
	void SaveScene(const char* newName = nullptr);

	bool HasChanges()const;
	bool isNewScene() const;
	
private:

	void SetupScene();

	static inline UID Validate(const UID id);

private:

	static RE_ECS_Pool scenePool;
	RE_ECS_Pool savedState;
	bool haschanges = false;

	eastl::stack<UID> to_delete;

	// Trees
	RE_AABBDynTree static_tree;
	RE_AABBDynTree dynamic_tree;

	RE_Scene* unsavedScene = nullptr;
	const char* currentScene = nullptr;
};

#endif // !__MODULESCENE_H__