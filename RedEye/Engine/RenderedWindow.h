#ifndef __RENDERED_WINDOW__
#define __RENDERED_WINDOW__

#include "EditorWindow.h"
#include "RE_Camera.h"

class RE_Camera;

class RenderedWindow : public EditorWindow
{
public:

	RenderedWindow(const char* name, bool start_active) : EditorWindow(name, start_active)
	{
		cam.SetupFrustum();
	}
	virtual ~RenderedWindow() = default;

	void UpdateViewPort()
	{
		cam.GetTargetViewPort(viewport);
		viewport.x = (width - viewport.z) * 0.5f;
		viewport.y = (heigth - viewport.w) * 0.5f + 20;
	}

	RE_Camera& GetCamera() { return cam; };
	const RE_Camera& GetCamera() const { return cam; };

	unsigned int GetWidth() const
	{
		int no_branch[2] = { 500, width };
		return static_cast<unsigned int>(no_branch[width > 0]);
	}

	unsigned int GetHeight() const
	{
		int no_branch[2] = { 500, heigth };
		return static_cast<unsigned int>(no_branch[heigth > 0]);
	}

	void Recalc() { recalc = true; }
	bool isSelected() const { return isWindowSelected; }

protected:

	RE_Camera cam;
	bool recalc = false;
	bool isWindowSelected = false;

	math::float4 viewport = math::float4::zero;
	int width = 0;
	int heigth = 0;
};

#endif //!__RENDERED_WINDOW__