#ifndef __MODULESCENE_H__
#define __MODULESCENE_H__

#include "Module.h"
#include "Event.h"
#include "QuadTree.h"

class RE_GameObject;

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
	RE_GameObject* AddGO(const char* name = nullptr, RE_GameObject* parent = nullptr);
	void AddGoToRoot(RE_GameObject* toAdd);

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

	void GetActive(std::vector<const RE_GameObject*>& objects) const;
	void GetActiveStatic(std::vector<const RE_GameObject*>& objects) const;
	void GetActiveNonStatic(std::vector<const RE_GameObject*>& objects) const;

	void UpdateQuadTree();

private:

	RE_GameObject* root = nullptr;

	QTree quad_tree;

	bool update_qt = true;
	bool static_gos_modified = false;
	bool scene_modified = false;

	std::string defaultModel;

	const char* sceneLoadedMD5 = nullptr;
};

#endif // !__MODULESCENE_H__