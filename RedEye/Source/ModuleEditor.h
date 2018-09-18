#ifndef __MODULEEDITOR__
#define __MODULEEDITOR__

#include "Module.h"
#include "ImGui\imgui.h"
#include <list>

class EditorWindow;
class ConsoleWindow;
class DemoWindow;
class ConfigWindow;
class HeriarchyWindow;
class PropertiesWindow;
class RandomTest;

union SDL_Event;

class ModuleEditor : public Module
{
public:
	ModuleEditor(const char* name, bool start_enabled = true);
	~ModuleEditor();

	bool Init(JSONNode* node = nullptr) override;
	update_status PreUpdate() override;
	update_status Update() override;
	bool CleanUp() override;

	//void DrawEditor() override;

	void AddTextConsole(const char* text);
	void Draw();
	void HandleSDLEvent(SDL_Event* e);

private:

	// Windows
	ConsoleWindow* console = nullptr;
	ConfigWindow* config = nullptr;
	HeriarchyWindow* heriarchy = nullptr;
	PropertiesWindow* properties = nullptr;
	RandomTest* rng = nullptr;

	std::list<EditorWindow*> windows;

	bool show_demo = false;
};

class EditorWindow
{
public:
	EditorWindow(const char* name, bool start_active);

	void DrawWindow();
	void SwitchActive();
	const char* Name() const;

	// Set position/size for window
	/*ImGui::SetNextWindowPos(ImVec2(0, 738));
	ImGui::SetWindowSize(ImVec2(1230.0f, 220.0f));*/

	inline operator bool() const;
	inline bool operator!() const;

protected:
	virtual void Draw() {}

	const char* name;
	bool active;
	bool lock_pos;
	ImVec2 pos;
	ImVec2 size;
	ImVec2 anchor;
};

class ConsoleWindow : public EditorWindow
{
public:
	ConsoleWindow(const char* name = "Console", bool start_active = true);
	void Draw() override;
	ImGuiTextBuffer console_buffer;
};

class ConfigWindow : public EditorWindow
{
public:
	ConfigWindow(const char* name = "Configuration", bool start_active = true);
	void Draw() override;
	bool changed_config;
};

class HeriarchyWindow : public EditorWindow
{
public:
	HeriarchyWindow(const char* name = "Heriarchy", bool start_active = false);
	void Draw() override;
};

class PropertiesWindow : public EditorWindow
{
public:
	PropertiesWindow(const char* name = "Properties", bool start_active = false);
	void Draw() override;
};

class RandomTest : public EditorWindow
{
public:
	RandomTest(const char* name = "Random Test", bool start_active = true);
	void Draw() override;

	int minInt = 0, maxInt = 1, resultInt = 0;
	float minF = 0.f, maxF = 1.f, resultF = 0.f;
};

/*/ Missing windows:
void GeometryTest();
void About();
void PlayPause();*/

#endif // !__MODULEEDITOR__