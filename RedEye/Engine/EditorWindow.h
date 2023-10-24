#ifndef __EDITOR_WINDOW__
#define __EDITOR_WINDOW__

#include <ImGui/imgui.h>

class EditorWindow
{
protected:

	const char* name = nullptr;
	bool active = true;
	bool lock_pos = false;
	ImVec2 pos;
	ImVec2 size;
	ImVec2 anchor;

public:

	EditorWindow(const char* name, bool start_active) : name(name), active(start_active) {}
	virtual ~EditorWindow() = default;

	const char* Name() const { return name; }
	bool IsActive() const { return active; }
	void ToggleActive() { active = !active; }

	void DrawWindow(bool draw_only_if_active = false, bool secondary = false)
	{
		if (draw_only_if_active && !active) return;

		if (lock_pos)
		{
			ImGui::SetNextWindowPos(pos);
			ImGui::SetWindowSize(size);
		}

		Draw(secondary);
	}

private:

	virtual void Draw(bool secondary = false) = 0;
};

#endif //!__EDITOR_WINDOW__