#ifndef __RENDERED_WINDOW__
#define __RENDERED_WINDOW__

#include "EditorWindow.h"
#include "RenderView.h"
#include "RE_Camera.h"

#include <EASTL/stack.h>
#include <EASTL/vector.h>

class RE_Component;
class RE_CompParticleEmitter;
class RE_Json;

class RenderedWindow : public EditorWindow
{
public:

	RenderView render_view;

protected:

	bool recalc = false;
	bool isWindowSelected = false;
	bool need_render = true;

	math::float4 viewport = math::float4::zero;
	int width = 0;
	int height = 0;

public:

	RenderedWindow(const char* name, bool start_active) : EditorWindow(name, start_active) {}
	virtual ~RenderedWindow() = default;

	virtual void RenderFBO() const;
	void DrawEditor();
	virtual void DrawDebug() const {}

	virtual RE_Camera& GetCamera() = 0;
	virtual const RE_Camera& GetCamera() const = 0;

	void Recalc() { recalc = true; }
	bool isSelected() const { return isWindowSelected; }

	void UpdateViewPort()
	{
		GetCamera().GetTargetViewPort(viewport);
		viewport.x = (width - viewport.z) * 0.5f;
		viewport.y = (height - viewport.w) * 0.5f + 20;
	}

	uint GetWidth() const
	{
		int no_branch[2] = { 500, width };
		return no_branch[width > 0];
	}

	uint GetHeight() const
	{
		int no_branch[2] = { 500, height };
		return no_branch[height > 0];
	}

	// Config Serialization
	void Load(RE_Json* node);
	void Save(RE_Json* node) const;

protected:

	void UpdateWindow();

	virtual void DrawEditor2() {}
	virtual eastl::stack<const RE_Component*> GatherDrawables() const;
	virtual eastl::vector<const RE_Component*> GatherSceneLights() const;
	virtual eastl::vector<const RE_CompParticleEmitter*> GatherParticleLights() const;

	virtual void Load2(RE_Json* node) {}
	virtual void Save2(RE_Json* node) const {}
};

class OwnCameraRenderedWindow : public RenderedWindow
{
protected:

	// Camera
	RE_Camera cam;
	float cam_speed = 25.0f;
	float cam_sensitivity = 0.01f;

	// Grid
	class RE_CompGrid* grid = nullptr;
	float grid_size[2];

public:

	OwnCameraRenderedWindow(const char* name, bool start_active);
	~OwnCameraRenderedWindow();

	virtual void DrawDebug() const override;

	// Camera
	void UpdateCamera();
	virtual void Orbit(float delta_x, float delta_y) {}
	virtual void Focus() {}
	RE_Camera& GetCamera() final { return cam; }
	const RE_Camera& GetCamera() const final { return cam; }

protected:

	void DrawEditor2() final;
	virtual void DrawEditor3() {}

	// Serialization
	void Load2(RE_Json* node) final;
	void Save2(RE_Json* node) const final;
	virtual void Load3(RE_Json* node) {}
	virtual void Save3(RE_Json* node) const {}
};

#endif //!__RENDERED_WINDOW__