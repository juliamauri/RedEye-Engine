#ifndef __MODULERENDER3D_H__
#define __MODULERENDER3D_H__

#include "RE_Math.h"
#include "Module.h"

#include <EASTL/stack.h>

class RE_CompCamera;

enum LightMode : int
{
	LIGHT_GL = 0,
	LIGHT_DIRECT,
	LIGHT_DEFERRED,
};

struct RenderMode
{
	RenderMode(LightMode mode, RE_CompCamera* camera, bool isGame = true, bool debug_draw = false, bool outline_selection = false, bool override_cull = false, bool skybox = true, bool wireframe = false, bool cull = true);

	LightMode light_mode = LIGHT_GL;
	RE_CompCamera* camera = nullptr;
	bool isGame = true;
	bool debug_draw = false;
	bool outline_selection = false;
	bool override_cull = true;
	bool skybox = true;
	bool wireframe = false;
	bool cull = true;
};

class ModuleRenderer3D : public Module 
{

public:
	ModuleRenderer3D(const char* name, bool start_enabled = true);
	~ModuleRenderer3D();

	bool Init(JSONNode* node) override;
	bool Start() override;
	update_status PreUpdate() override;
	update_status PostUpdate() override;
	bool CleanUp() override;

	void RecieveEvent(const Event& e) override;
	void DrawEditor() override;

	bool Load(JSONNode* node) override;
	bool Save(JSONNode* node) const override;

	// Editor Values
	void SetVSync(bool enable);
	void SetDepthTest(bool enable);
	void SetFaceCulling(bool enable);
	void SetLighting(bool enable);
	void SetTexture2D(bool enable);
	void SetColorMaterial(bool enable);
	void SetWireframe(bool enable);

	// Shaders - A vector in GLSL contains 4 component
	unsigned int GetMaxVertexAttributes(); //it's usually 16

	// Draws
	void DirectDrawCube(math::vec position, math::vec color);

	// Context & Viewport
	void* GetWindowContext()const;

	void ChangeFBOSize(int width, int height, bool editor = false);

	unsigned int GetRenderedEditorSceneTexture()const;
	unsigned int GetRenderedGameSceneTexture()const;

	void ReRenderThumbnail(const char* res);

private:

	void DrawScene(const RenderMode& mode);

public:

	static LightMode render_pass;

private:

	eastl::stack<const char*> thumbnailsToRender;

	// Context
	void* mainContext;

	// Configuration
	bool vsync = false;
	bool cullface = false;
	bool depthtest = true;
	bool lighting = false;
	bool texture2d = false;
	bool color_material = false;

	bool wireframe_scene = false;
	bool cull_scene = true;
	bool deferred_light = false;

	// FBOs
	unsigned int sceneEditorFBO = 0;
	unsigned int sceneGameFBO = 0;

	unsigned int deferredEditorFBO = 0;
	unsigned int lightPassFBO = 0;
	//unsigned int deferredGameFBO = 0;
};

#endif // !__MODULERENDER3D_H__