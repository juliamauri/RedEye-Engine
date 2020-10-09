#ifndef __MODULERENDER3D_H__
#define __MODULERENDER3D_H__

#include "Module.h"
#include <EASTL/stack.h>

enum LightMode : int
{
	LIGHT_DISABLED = 0,
	LIGHT_GL,
	LIGHT_DIRECT,
	LIGHT_DEFERRED,
};

enum RenderViewFlags : short
{
	FRUSTUM_CULLING = 0x1,	 // 000000000001
	OVERRIDE_CULLING = 0x2,	 // 000000000010
	OUTLINE_SELECTION = 0x4, // 000000000100
	DEBUG_DRAW = 0x8,		 // 000000001000
	SKYBOX = 0x10,			 // 000000010000
	BLENDED = 0x20,			 // 000000100000

	WIREFRAME = 0x40,		 // 000001000000
	FACE_CULLING = 0X80,	 // 000010000000
	TEXTURE_2D = 0x100,		 // 000100000000
	COLOR_MATERIAL = 0x200,	 // 001000000000
	DEPTH_TEST = 0x400		 // 010000000000
};

class RE_CompCamera;

struct RenderView
{
	RenderView(eastl::string name = "",
		eastl::pair<unsigned int, unsigned int> fbos = { 0, 0 },
		short flags = 0, LightMode light = LIGHT_GL);

	eastl::string name;
	eastl::pair<unsigned int, unsigned int> fbos;
	short flags;
	LightMode light;
	float clear_color[4];
	RE_CompCamera* camera = nullptr;

	const unsigned int GetFBO() const;

	static const char* labels[11];
};

enum RENDER_VIEWS : short
{
	VIEW_EDITOR,
	VIEW_GAME,
	VIEW_OTHER
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

	// Context & Viewport
	void* GetWindowContext()const;

	// Shaders - A vector in GLSL contains 4 component
	unsigned int GetMaxVertexAttributes(); //it's usually 16

	// Sets shader for unassigned geometry
	static const LightMode GetLightMode();

	void ChangeFBOSize(int width, int height, bool editor = false);
	unsigned int GetRenderedEditorSceneTexture()const;
	unsigned int GetRenderedGameSceneTexture()const;

	// Thumbnail
	void ReRenderThumbnail(const char* res);

private:

	void DrawScene(RenderView& render_view);

	void inline SetupShaders();

	void inline SetWireframe(bool enable);
	void inline SetFaceCulling(bool enable);
	void inline SetTexture2D(bool enable);
	void inline SetColorMaterial(bool enable);
	void inline SetDepthTest(bool enable);
	void inline SetLighting(bool enable);

	void DrawQuad();
	void DirectDrawCube(math::vec position, math::vec color);

private:

	// Scene Drawing
	float time, dt;
	eastl::vector<const char*> activeShaders;

	// Context
	void* mainContext;

	// Renderer Flags
	bool vsync = false;

	bool wireframe = false;
	bool cullface = false;
	bool texture2d = false;
	bool color_material = false;
	bool depthtest = false;
	bool lighting = false;

	static LightMode current_lighting;

	// Rendering Views
	eastl::vector<RenderView> render_views;

	// Thumbnails
	eastl::stack<const char*> thumbnailsToRender;
};

#endif // !__MODULERENDER3D_H__