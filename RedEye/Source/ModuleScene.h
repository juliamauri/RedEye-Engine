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

	void Clear();

	eastl::map< RE_GameObject *,int> PushWithChilds(RE_GameObject* val, bool root = true);
	int Push(RE_GameObject* val)override;

	eastl::vector< RE_GameObject*> PopWithChilds(int id, bool root = true);
	RE_GameObject* Pop(int id)override;

	int WhatID(RE_GameObject* go)const;

private:
	eastl::map< RE_GameObject*, int> goToID;

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

	void DrawTrees() const;
	RE_GameObject* RayCastSelect(math::Ray& ray);
	void FustrumCulling(eastl::vector<const RE_GameObject*>& container, const math::Frustum& frustum);

	void Serialize();
	void LoadFBXOnScene(const char* fbxPath);
	void LoadTextureOnSelectedGO(const char* texturePath);

private:

	void GetActive(eastl::list<RE_GameObject*>& objects) const;
	void GetActiveStatic(eastl::list<RE_GameObject*>& objects) const;
	void GetActiveNonStatic(eastl::list<RE_GameObject*>& objects) const;

	void ResetTrees();

private:

	RE_GameObject* savedState = nullptr;
	RE_GameObject* root = nullptr;

	GameObjectManager goManager;

	AABBDynamicTree static_tree;
	AABBDynamicTree dynamic_tree;

	eastl::string defaultModel;

	const char* sceneLoadedMD5 = nullptr;
};

#endif // !__MODULESCENE_H__