#ifndef __MODULESCENE_H__
#define __MODULESCENE_H__

#include "Module.h"
#include "Event.h"
#include "QuadTree.h"
#include "RE_GOManager.h"

class RE_GameObject;
class RE_Scene;

class ModuleScene : public Module
{
public:
	ModuleScene(const char* name = "Scene", bool start_enabled = true);
	~ModuleScene();

	bool Start() override;
	update_status Update() override;
	update_status PostUpdate() override;
	bool CleanUp() override;

	// Draws
	void DrawEditor() override;
	void DebugDraw() const;
	
	// Events
	void OnPlay() override;
	void OnPause() override;
	void OnStop() override;
	void RecieveEvent(const Event& e) override;

	// Current Pool
	static RE_GOManager* GetScenePool();
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

	void AddGOPool(RE_GOManager* toAdd);

	// Scene Gameobject Filtering
	UID RayCastSelect(math::Ray& global_ray);
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

	// Trees
	/*void GetActive(eastl::list<RE_GameObject*>& objects) const;
	void GetActiveStatic(eastl::list<RE_GameObject*>& objects) const;
	void GetActiveNonStatic(eastl::list<RE_GameObject*>& objects) const;*/

	void SetupScene();

	static inline UID Validate(const UID id);

private:

	static RE_GOManager scenePool;
	RE_GOManager savedState;
	bool haschanges;

	eastl::stack<UID> to_delete;

	// Trees
	AABBDynamicTree static_tree;
	AABBDynamicTree dynamic_tree;

	RE_Scene* unsavedScene = nullptr;
	const char* currentScene = nullptr;

};

#endif // !__MODULESCENE_H__