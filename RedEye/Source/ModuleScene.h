#ifndef __MODULESCENE_H__
#define __MODULESCENE_H__

#include "Module.h"
#include "Event.h"
#include "QuadTree.h"

#include <windows.h>

class RE_GameObject;
class RE_Scene;



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
	void AddGoToRoot(RE_GameObject* toAdd);

	void CreateCube(RE_GameObject* parent = nullptr);
	void CreateDodecahedron(RE_GameObject* parent = nullptr);
	void CreateTetrahedron(RE_GameObject* parent = nullptr);
	void CreateOctohedron(RE_GameObject* parent = nullptr);
	void CreateIcosahedron(RE_GameObject* parent = nullptr);
	void CreatePlane(RE_GameObject* parent = nullptr);
	void CreateSphere(RE_GameObject* parent = nullptr);
	void CreateCylinder(RE_GameObject* parent = nullptr);
	void CreateHemiSphere(RE_GameObject* parent = nullptr);
	void CreateTorus(RE_GameObject* parent = nullptr);
	void CreateTrefoilKnot(RE_GameObject* parent = nullptr);
	void CreateRock(RE_GameObject* parent = nullptr);
	void CreateCamera(RE_GameObject* parent = nullptr);

	void DrawEditor() override;

	void DrawTrees() const;
	RE_GameObject* RayCastSelect(math::Ray& ray);
	void FustrumCulling(eastl::vector<const RE_GameObject*>& container, const math::Frustum& frustum);

	void NewEmptyScene(const char* name = "New Scene");

	void LoadScene(const char* sceneMD5, bool ignorehandle = false);
	void SaveScene(const char* newName = nullptr);
	const char* GetCurrentScene()const;
	void ClearScene();

	void AddGameobject(RE_GameObject* toAdd);

	bool HasChanges()const;
	bool isNewScene() const;
	
private:

	void GetActive(eastl::list<RE_GameObject*>& objects) const;
	void GetActiveStatic(eastl::list<RE_GameObject*>& objects) const;
	void GetActiveNonStatic(eastl::list<RE_GameObject*>& objects) const;

	void ResetTrees();

	void SetupScene();

private:

	RE_GameObject* savedState = nullptr;
	RE_GameObject* root = nullptr;


	AABBDynamicTree static_tree;
	AABBDynamicTree dynamic_tree;

	RE_Scene* unsavedScene = nullptr;
	const char* currentScene = nullptr;

	bool haschanges = false;
};

#endif // !__MODULESCENE_H__