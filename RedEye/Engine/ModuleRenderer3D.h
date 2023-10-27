#ifndef __MODULERENDER3D_H__
#define __MODULERENDER3D_H__

#include "EventListener.h"
#include "RenderView.h"
#include "RE_Camera.h"
#include "RE_ParticleEmitter.h"
#include <EASTL/stack.h>
#include <EASTL/vector.h>

class RenderedWindow;
class RE_Component;
class RE_CompParticleEmitter;
class RE_SkyBox;

class ModuleRenderer3D : public EventListener 
{
public:

	enum class Flag : ushort
	{
		VSYNC = 1 << 0,				// 0000 0000 0000 0001
		SHARE_LIGHT_PASS = 1 << 1	// 0000 0000 0000 0010
	};

private:

	// OpenGL
	static void* mainContext;

	// Current State
	static ushort flags;
	static RenderView::LightMode current_lighting;
	static uint current_fbo;
	static eastl::vector<const char*> activeShaders;

	//Debug info
	static uint lightsCount;
	static uint particlelightsCount;

	// Utility
	float point_size = 2.f;
	eastl::vector<math::float2> circle_precompute;

public:

	ModuleRenderer3D() = default;
	~ModuleRenderer3D() final = default;

	bool Init();
	void PostUpdate();
	void CleanUp();
	void DrawEditor();
	void RecieveEvent(const Event& e) final;
	void Load();
	void Save() const;

	// Actions
	static void LoadCameraMatrixes(const RE_Camera& camera);
	void DrawScene(
		const RenderView& render_view,
		const RE_Camera& camera,
		eastl::stack<const RE_Component*>& drawables,
		eastl::vector<const RE_Component*> lights,
		eastl::vector<const RE_CompParticleEmitter*> particle_lights,
		RenderedWindow* toDebug = nullptr) const;

	// Draws
	void DrawStencil(GO_UID stencilGO, bool has_depth_test) const;
	void DebugDrawParticleEmitter(const RE_ParticleEmitter& sim) const;

	// Getters
	static uint GetDepthTexture();
	static RenderView::LightMode GetLightMode() { return current_lighting; }
	static eastl::vector<const char*>& GetActiveShaders() { return activeShaders; }

	// OpenGL
	static void* GetWindowContext() { return mainContext; }
	static const char* GetGPURenderer();
	static const char* GetGPUVendor();
	static uint GetMaxVertexAttributes(); // (usually 16 as vector in GLSL contains 4 component)

private:

	// OpenGL
	bool InitializeGL();

	// Utility
	void PrecomputeCircle(int steps = 12);

	// Render Flags
	static bool HasFlag(RenderView::Flag flag);
	static void AddFlag(RenderView::Flag flag, bool force_state = false);
	static void RemoveFlag(RenderView::Flag flag, bool force_state = false);
	static void SetupFlag(RenderView::Flag flag, bool target_state, bool force_state = false);
	static void SetupFlags(ushort flags, bool force_state = false);

	// Prepare To Render
	void PullActiveShaders() const;
	void PrepareToRender(const RenderView& render_view, const RE_Camera& camera) const;
	void CategorizeDrawables(
		eastl::stack<const RE_Component*>& to_draw,
		eastl::stack<const RE_Component*>& geo,
		eastl::stack<const RE_Component*>& blended_geo,
		eastl::stack<const RE_CompParticleEmitter*>& particle_systems,
		eastl::stack<const RE_CompParticleEmitter*>& blended_particle_systems) const;

	// Scene Draws
	void DrawSceneDeferred(
		const RenderView& render_view,
		const RE_Camera& camera,
		eastl::stack<const RE_Component*>& blended_geo,
		eastl::stack<const RE_CompParticleEmitter*>& blended_particle_systems,
		eastl::vector<const RE_Component*> lights,
		eastl::vector<const RE_CompParticleEmitter*> particle_lights) const;
	// Utility Draws
	void DrawDebug(
		const RenderedWindow* toDebug,
		const RenderView& render_view,
		const RE_Camera& camera) const;
	
	void DrawSkyBox(const RE_SkyBox* skybox) const;

	// Direct Draws
	void DrawQuad() const;
	void DirectDrawCube(math::vec position, math::vec color) const;
	void DrawAASphere(const math::vec p_pos, const float radius) const;

	// Particle Editor Draws
	void DrawParticleEditor(
		RenderView& render_view,
		const RE_Camera& camera) const;

	void DrawParticleEditorDebug(
		const RenderView& render_view,
		const RE_ParticleEmitter* emitter,
		const RE_Camera& camera) const;

	void DrawParticleLights(const uint sim_id) const;
};

#endif // !__MODULERENDER3D_H__