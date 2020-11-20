#ifndef __MODULERENDER3D_H__
#define __MODULERENDER3D_H__

#include "Module.h"
#include "RE_FBOManager.h"
#include <EASTL/stack.h>
#include "MathGeoLib/include/Math/float4.h"

class RE_Material;
class RE_SkyBox;

enum LightMode : int
{
	LIGHT_DISABLED = 0,
	LIGHT_GL,
	LIGHT_DIRECT,
	LIGHT_DEFERRED,
};

enum RenderViewFlags : short
{
	FRUSTUM_CULLING = 0x1,	 // 0000 0000 0001
	OVERRIDE_CULLING = 0x2,	 // 0000 0000 0010
	OUTLINE_SELECTION = 0x4, // 0000 0000 0100
	DEBUG_DRAW = 0x8,		 // 0000 0000 1000

	SKYBOX = 0x10,			 // 0000 0001 0000
	BLENDED = 0x20,			 // 0000 0010 0000
	WIREFRAME = 0x40,		 // 0000 0100 0000
	FACE_CULLING = 0X80,	 // 0000 1000 0000

	TEXTURE_2D = 0x100,		 // 0001 0000 0000
	COLOR_MATERIAL = 0x200,	 // 0010 0000 0000
	DEPTH_TEST = 0x400,		 // 0100 0000 0000
	CLIP_DISTANCE = 0x800	 // 1000 0000 0000
};

class RE_CompCamera;

struct RenderView
{
	RenderView(eastl::string name = "",
		eastl::pair<unsigned int, unsigned int> fbos = { 0, 0 },
		short flags = 0, LightMode light = LIGHT_GL, math::float4 clipDistance = math::float4::zero);

	eastl::string name;
	eastl::pair<unsigned int, unsigned int> fbos;
	short flags;
	LightMode light;
	math::float4 clear_color;
	math::float4 clip_distance;
	RE_CompCamera* camera = nullptr;

	const unsigned int GetFBO() const;

	static const char* labels[12];
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
	ModuleRenderer3D(const char* name = "Renderer3D", bool start_enabled = true);
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

	// GPU Specs
	const char* GetGPURenderer() const;
	const char* GetGPUVendor() const;

	// Sets shader for unassigned geometry
	static const LightMode GetLightMode();

	void ChangeFBOSize(int width, int height, bool editor = false);
	unsigned int GetRenderedEditorSceneTexture()const;
	unsigned int GetDepthTexture()const;
	unsigned int GetRenderedGameSceneTexture()const;

	void PushSceneRend(RenderView& rV);
	void PushThumnailRend(const char* md5, bool redo = false);

public:
	enum RenderType {
		R_R_SCENE,
		T_R_GO,
		T_R_MAT,
		T_R_TEX,
		T_R_SKYBOX
	};
	struct RenderQueue {
		RenderQueue(RenderType t, RenderView& rv, const char* r, bool re = false) : type(t), renderview(rv), resMD5(r), redo(re) {}
		RenderType type;
		RenderView& renderview;
		const char* resMD5;
		bool redo = false;
	};

private:

	void DrawScene(const RenderView& render_view);
	void ThumbnailGameObject(RE_GameObject* go);
	void ThumbnailMaterial(RE_Material* mat);
	void ThumbnailSkyBox(RE_SkyBox* skybox);

	// Render Flags
	void inline SetWireframe(bool enable);
	void inline SetFaceCulling(bool enable);
	void inline SetTexture2D(bool enable);
	void inline SetColorMaterial(bool enable);
	void inline SetDepthTest(bool enable);
	void inline SetLighting(bool enable);
	void inline SetClipDistance(bool enable);

	// Direct Draws
	void DrawQuad();
	void DirectDrawCube(math::vec position, math::vec color);

public:

	static RE_FBOManager fbomanager;

private:

	eastl::stack<RenderQueue> rendQueue;

	// Scene Drawing
	//float time, dt;
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
	bool clip_distance = false;

	static LightMode current_lighting;
	static unsigned int current_fbo;

	// Rendering Views
	eastl::vector<RenderView> render_views;

	//Thumbnail values
	RenderView thumbnailView;
	unsigned int mat_vao = 0, mat_vbo = 0, mat_ebo = 0, mat_triangles = 0;
};

#endif // !__MODULERENDER3D_H__