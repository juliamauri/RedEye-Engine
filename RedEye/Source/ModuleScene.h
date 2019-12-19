#ifndef __MODULESCENE_H__
#define __MODULESCENE_H__

#include "Module.h"
#include "Event.h"
#include "QuadTree.h"
#include "PoolMapped.h"

#include <windows.h>

class RE_GameObject;

class GameObjectManager : public PoolMapped<RE_GameObject*, int> {
public:
	GameObjectManager() { }
	~GameObjectManager() { }

	std::map< RE_GameObject *,int> PushWithChilds(RE_GameObject* val, bool root = true);
	int Push(RE_GameObject* val)override;

};


class ModuleScene : public Module
{
public:
	ModuleScene(const char* name, bool start_enabled = true);
	~ModuleScene();

	bool Init(JSONNode* node) override;
	bool Start() override;
	update_status Update() override;
	bool CleanUp() override;

	void OnPlay() override;
	void OnPause() override;
	void OnStop() override;

	void RecieveEvent(const Event& e) override;

	RE_GameObject* GetRoot() const;
	const RE_GameObject* GetRoot_c() const;
	RE_GameObject* AddGO(const char* name = nullptr, RE_GameObject* parent = nullptr, bool broadcast = true);
	void AddGoToRoot(RE_GameObject* toAdd);

	void CreatePlane();
	void CreateCube();
	void CreateSphere();
	void CreateCamera();

	void DrawEditor() override;

	void DrawQTree() const;
	RE_GameObject* RayCastSelect(math::Ray& ray);
	void FustrumCulling(std::vector<const RE_GameObject*>& container, math::Frustum& frustum);

	void Serialize();
	void LoadFBXOnScene(const char* fbxPath);
	void LoadTextureOnSelectedGO(const char* texturePath);


private:

	void GetActive(std::list<RE_GameObject*>& objects) const;
	void GetActiveStatic(std::list<RE_GameObject*>& objects) const;
	void GetActiveNonStatic(std::list<RE_GameObject*>& objects) const;

	void UpdateQuadTree();

private:

	RE_GameObject* savedState = nullptr;
	RE_GameObject* root = nullptr;

	AABBDynamicTree dynamic_tree;
	GameObjectManager goManager;

	bool update_qt = false;
	bool static_gos_modified = false;
	bool scene_modified = false;

	std::list<RE_GameObject*> active_static_gos;
	std::list<RE_GameObject*> active_non_static_gos;
	std::list<RE_GameObject*> tree_free_static_gos;

	std::string defaultModel;

	const char* sceneLoadedMD5 = nullptr;
};

#endif // !__MODULESCENE_H__