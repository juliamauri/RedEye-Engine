#ifndef __MODULERENDER3D_H__
#define __MODULERENDER3D_H__

#include "EventListener.h"
#include "RenderView.h"
#include <EASTL/stack.h>

class ModuleRenderer3D : public EventListener 
{
public:
	ModuleRenderer3D();
	~ModuleRenderer3D();

	bool Init();
	bool Start();
	void PostUpdate();
	void CleanUp();

	void DrawEditor();
	void RecieveEvent(const Event& e) override;

	void Load();
	void Save() const;

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
	static RenderView::LightMode GetLightMode();
	
	static RE_CompCamera* GetCamera();

	void ChangeFBOSize(int width, int height, RenderView::Type view);
	uint GetRenderViewTexture(RenderView::Type type) const;
	unsigned int GetDepthTexture() const;

	void PushSceneRend(RenderView& rV);
	void PushThumnailRend(const char* md5, bool redo = false);

	math::float4 GetRenderViewClearColor(RenderView::Type r_view) const;
	RenderView::LightMode GetRenderViewLightMode(RenderView::Type r_view) const;
	bool GetRenderViewDebugDraw(RenderView::Type r_view) const;
	void SetRenderViewDeferred(RenderView::Type r_view, bool using_deferred);
	void SetRenderViewClearColor(RenderView::Type r_view, math::float4 clear_color);
	void SetRenderViewDebugDraw(RenderView::Type r_view, bool debug_draw);

	enum class RenderType : ushort
	{
		SCENE,
		GO,
		MAT,
		TEX,
		SKYBOX
	};
	
	struct RenderQueue
	{
		RenderType type;
		RenderView& renderview;
		const char* resMD5;
		bool redo = false;
		RenderQueue(RenderType t, RenderView& rv, const char* r, bool re = false) :
			type(t), renderview(rv), resMD5(r), redo(re) {}
	};

private:

	void DrawScene(const RenderView& render_view);
	void DrawDebug(const RenderView& render_view);
	void DrawParticleEditor(RenderView& render_view);
	void DrawParticleLights(const uint sim_id);
	void DrawSkyBox();
	void DrawStencil(class RE_GameObject* go, class RE_Component* comp, bool has_depth_test);

	void ThumbnailGameObject(RE_GameObject* go);
	void ThumbnailMaterial(class RE_Material* mat);
	void ThumbnailSkyBox(class RE_SkyBox* skybox);

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

	class RE_FBOManager* fbos = nullptr;

private:

	eastl::stack<RenderQueue> render_queue;
	eastl::vector<const char*> activeShaders;

	// Context
	void* mainContext = nullptr;

	// Renderer Flags
	bool vsync = false;

	bool wireframe = false;
	bool cullface = false;
	bool texture2d = false;
	bool color_material = false;
	bool depthtest = false;
	bool lighting = false;
	bool clip_distance = false;

	static RenderView::LightMode current_lighting;
	static RE_CompCamera* current_camera;
	static unsigned int current_fbo;

	// Rendering Views
	eastl::vector<RenderView> render_views;

	//Thumbnail values
	RenderView thumbnailView;
	unsigned int mat_vao = 0, mat_vbo = 0, mat_ebo = 0, mat_triangles = 0;

	//Debug info
	unsigned int lightsCount = 0;
	unsigned int particlelightsCount = 0;

	// Light pass render
	bool shareLightPass = false;
};

#endif // !__MODULERENDER3D_H__