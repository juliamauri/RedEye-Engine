#ifndef __MODULEEDITOR__
#define __MODULEEDITOR__

#include "Module.h"
#include "ImGui\imgui.h"
#include <map>

union SDL_Event;

class ModuleEditor : public Module
{
public:
	ModuleEditor(const char* name, bool start_enabled = true);
	~ModuleEditor();

	bool Start() override;
	update_status PreUpdate() override;
	update_status Update() override;
	bool CleanUp() override;

	//void DrawEditor() override;

	void AddTextConsole(const char* text);
	void Draw();
	void HandleSDLEvent(SDL_Event* e);

private:

	ImGuiTextBuffer console_buffer;

};
#endif // !__MODULEEDITOR__