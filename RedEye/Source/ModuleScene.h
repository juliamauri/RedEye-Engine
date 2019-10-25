#ifndef __MODULESCENE_H__
#define __MODULESCENE_H__

#include "Module.h"
#include "Event.h"
#include "QuadTree.h"

class RE_GameObject;
class RE_Mesh;

class ModuleScene : public Module
{
public:
	ModuleScene(const char* name, bool start_enabled = true);
	~ModuleScene();

	bool Start() override;
	update_status Update() override;
	bool CleanUp() override;

	void OnPlay() override;
	void OnPause() override;
	void OnStop() override;

	void RecieveEvent(const Event& e) override;

	RE_GameObject* AddGO(const char* name = nullptr, RE_GameObject* parent = nullptr);
	void AddGoToRoot(RE_GameObject* toAdd);
	void DuplicateSelectedObject();

	void CreateCube();
	void CreateSphere();

	void DrawEditor() override;
	void DrawScene();
	void DrawHeriarchy();
	void DrawFocusedProperties();

	void SetSelected(RE_GameObject* selected);
	RE_GameObject* GetSelected() const;
	void RayCastSelect(math::Ray& ls);

	void Serialize();
	void LoadFBXOnScene(const char* fbxPath);
	void SceneModified();

	//shaders
	unsigned int modelloading;

	// Checkers
	unsigned int checkers_texture;

private:

	RE_GameObject* root = nullptr;
	RE_GameObject* selected = nullptr;

	// Bounding Boxes
	bool aabb_need_reset = false;
	unsigned int aabb_reset_time = 0;

	// Quadtree
	QTree quad_tree;
	bool draw_quad_tree = false;

	// Config
	bool draw_all_aabb = true;
	math::vec all_aabb_color;
	float all_aabb_width = 1.0f;

	bool draw_selected_aabb = true;
	math::vec sel_aabb_color;
	float sel_aabb_width = 5.0f;

	bool focus_on_select = false;

	// Particles
	RE_Mesh* smoke_particle = nullptr;
	unsigned int shader_particle;
};


#endif // !__MODULESCENE_H__