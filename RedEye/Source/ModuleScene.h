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

	void FileDrop(const char* file);
	void RecieveEvent(const Event& e) override;

	RE_GameObject* AddGO(const char* name = nullptr, RE_GameObject* parent = nullptr);
	void AddGoToRoot(RE_GameObject* toAdd);
	void DuplicateSelectedObject();

	void DrawEditor() override;
	void DrawScene();
	void DrawHeriarchy();
	void DrawFocusedProperties();

	void SetSelected(RE_GameObject* selected);
	RE_GameObject* GetSelected() const;

	void RayCastSelect(math::Ray& ls);

	void Serialize();


	//shaders
	unsigned int modelloading;

private:
	RE_GameObject* root = nullptr;
	RE_GameObject* selected = nullptr;

	bool draw_quad_tree = false;
	QTree quad_tree;

	int drawn_go = 0;

	RE_Mesh* smoke_particle = nullptr;
	unsigned int shader_particle;
};


#endif // !__MODULESCENE_H__