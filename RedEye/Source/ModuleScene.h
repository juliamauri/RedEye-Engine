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

	RE_GameObject* AddGO(const char* name = nullptr, RE_GameObject* parent = nullptr);
	void AddGoToRoot(RE_GameObject* toAdd);
	void DuplicateSelectedObject();

	void CreateCube();
	void CreateSphere();
	void CreateCamera();

	void DrawEditor() override;
	void DrawScene();
	void DrawHeriarchy();
	void DrawFocusedProperties();

	void SetSelected(RE_GameObject* selected);
	RE_GameObject* GetSelected() const;
	void RayCastSelect(math::Ray& ls);

	void Serialize();
	void LoadFBXOnScene(const char* fbxPath);
	void LoadTextureOnSelectedGO(const char* texturePath);
	
	void StaticTransformed();
	std::list<RE_CompCamera*> GetCameras();

	uint GetShaderScene() const;

	bool DrawingSelAABB() const;

private:
	//init values
	std::string defaultModel;

	RE_GameObject* root = nullptr;
	RE_GameObject* selected = nullptr;

	// Bounding Boxes
	bool static_gos_modified = false;
	unsigned int aabb_reset_time = 0;

	bool draw_all_aabb = false;
	math::vec all_aabb_color;

	bool draw_selected_aabb = false;
	math::vec sel_aabb_color;

	// Quadtree
	QTree quad_tree;
	bool draw_quad_tree = true;
	math::vec quad_tree_color;

	// Camera Frustums
	bool draw_cameras = true;
	math::vec frustum_color;

	// Config
	bool focus_on_select = false;

	// Shaders
	unsigned int sceneShader = 0;
	unsigned int skyboxShader = 0;
};


#endif // !__MODULESCENE_H__