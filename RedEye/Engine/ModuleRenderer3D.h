#ifndef __MODULERENDER3D_H__
#define __MODULERENDER3D_H__

#include "EventListener.h"
#include "RenderView.h"
#include <EASTL/stack.h>

class RE_Component;
class RE_CompParticleEmitter;
class RE_SkyBox;

class ModuleRenderer3D : public EventListener 
{
public:
	ModuleRenderer3D();
	~ModuleRenderer3D() final;

	bool Init();
	bool Start();
	void PostUpdate();
	void CleanUp();
	void DrawEditor();
	void RecieveEvent(const Event& e) final;

	// Config Serialization
	void Load();
	void Save() const;

	// Actions
	void LoadCameraMatrixes(const RE_Camera& camera);
	void ChangeFBOSize(int width, int height, RenderView::Type view);
	void PushThumnailRend(const char* md5, bool redo = false);

	// Setters
	void SetRenderViewDeferred(RenderView::Type r_view, bool using_deferred);
	void SetRenderViewClearColor(RenderView::Type r_view, math::float4 clear_color);
	void SetRenderViewDebugDraw(RenderView::Type r_view, bool debug_draw);

	// Getters
	void* GetWindowContext() const;
	static RenderView::LightMode GetLightMode();
	static RE_Camera* GetCamera();

	const char* GetGPURenderer() const;
	const char* GetGPUVendor() const;
	uint GetMaxVertexAttributes(); // (usually 16 as vector in GLSL contains 4 component)

	uintptr_t GetRenderViewTexture(RenderView::Type type) const;
	uint GetDepthTexture() const;

	// I don't like these
	math::float4 GetRenderViewClearColor(RenderView::Type r_view) const;
	RenderView::LightMode GetRenderViewLightMode(RenderView::Type r_view) const;
	bool GetRenderViewDebugDraw(RenderView::Type r_view) const;

private:

	// Render Flags
	inline bool HasFlag(RenderView::Flag flag) const;
	void AddFlag(RenderView::Flag flag, bool force_state = false);
	void RemoveFlag(RenderView::Flag flag, bool force_state = false);
	void SetupFlag(RenderView::Flag flag, bool target_state, bool force_state = false);
	void SetupFlags(ushort flags, bool force_state = false);

	// Prepare To Render
	void PrepareToRender(const RenderView& render_view);
	void CategorizeDrawables(eastl::stack<const RE_Component*>& to_draw,
		eastl::stack<const RE_Component*>& geo,
		eastl::stack<const RE_Component*>& blended_geo,
		eastl::stack<const RE_CompParticleEmitter*>& particle_systems,
		eastl::stack<const RE_CompParticleEmitter*>& blended_particle_systems);

	eastl::stack<const RE_Component*> GatherDrawables(const RenderView& render_view) const;
	eastl::vector<const RE_Component*> GatherSceneLights() const;
	eastl::vector<const class RE_CompParticleEmitter*> GatherParticleLights() const;

	// Main Draws
	void DrawScene(const RenderView& render_view);
	void DrawSceneForward(
		const RenderView& render_view,
		eastl::stack<const RE_Component*>& blended_geo,
		eastl::stack<const RE_CompParticleEmitter*>& blended_particle_systems);
	void DrawSceneDeferred(
		const RenderView& render_view,
		eastl::stack<const RE_Component*>& blended_geo,
		eastl::stack<const RE_CompParticleEmitter*>& blended_particle_systems);

	// Utility Draws
	void DrawDebug(const RenderView& render_view);
	void DrawSkyBox(const RE_SkyBox* skybox);
	void DrawStencil(bool has_depth_test);

	// Direct Draws
	void DrawQuad() const;
	void DirectDrawCube(math::vec position, math::vec color) const;

	// Particle Editor Draws
	void DrawParticleEditor(RenderView& render_view);
	void DrawParticleEditorDebug(const RenderView& render_view, const class RE_ParticleEmitter* emitter);
	void DrawParticleLights(const uint sim_id);

	// Thumbnail Draws
	void ThumbnailGameObject(RE_GameObject* go);
	void ThumbnailMaterial(class RE_Material* mat);
	void ThumbnailSkyBox(RE_SkyBox* skybox);

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

public:

	class RE_FBOManager* fbos = nullptr;

private:

	// Rendering Data
	void* mainContext = nullptr;

	static RenderView::LightMode current_lighting;
	static RE_Camera* current_camera;
	static uint current_fbo;

	eastl::vector<RenderView> render_views;
	eastl::stack<RenderQueue> render_queue;
	eastl::vector<const char*> activeShaders;

	// Renderer Flags
	ushort flags = 0;

	//Thumbnail values
	RenderView thumbnailView;
	uint mat_vao = 0;
	uint mat_vbo = 0;
	uint mat_ebo = 0;
	uint mat_triangles = 0;

	//Debug info
	uint lightsCount = 0;
	uint particlelightsCount = 0;
};

#endif // !__MODULERENDER3D_H__