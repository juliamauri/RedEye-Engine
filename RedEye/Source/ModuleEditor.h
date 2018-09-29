#ifndef __MODULEEDITOR__
#define __MODULEEDITOR__

#include "Module.h"
#include "ImGui\imgui.h"
#include <list>
#include <string>

class EditorWindow;
class ConsoleWindow;
class DemoWindow;
class ConfigWindow;
class HeriarchyWindow;
class PropertiesWindow;
class AboutWindow;
class RandomTest;
class RendererTest;
class GeometryTest;

union SDL_Event;

struct SoftwareInfo
{
	SoftwareInfo(const char * name, const char * version = nullptr, const char * website = nullptr);
	std::string name, version, website;
};


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
	bool AddSoftwareUsed(SoftwareInfo s);
	void Draw();
	void HandleSDLEvent(SDL_Event* e);

private:

	// Windows
	ConsoleWindow* console = nullptr;
	ConfigWindow* config = nullptr;
	HeriarchyWindow* heriarchy = nullptr;
	PropertiesWindow* properties = nullptr;
	AboutWindow* about = nullptr;

	RandomTest* rng = nullptr;
	RendererTest* renderer = nullptr;
	GeometryTest* geo_test = nullptr;

	std::list<EditorWindow*> windows, tools;

	bool show_all = true;
	bool show_demo = false;
};

class EditorWindow
{
public:
	EditorWindow(const char* name, bool start_active);
	virtual ~EditorWindow();
	void DrawWindow();
	void SwitchActive();
	const char* Name() const;

	// Set position/size for window
	/*ImGui::SetNextWindowPos(ImVec2(0, 738));
	ImGui::SetWindowSize(ImVec2(1230.0f, 220.0f));*/

	bool IsActive() const;
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
	bool scroll_to_bot = true;
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

class AboutWindow : public EditorWindow
{
public:
	AboutWindow(const char* name = "About", bool start_active = true);
	void Draw() override;
	std::list<SoftwareInfo> sw_info;
};

class RandomTest : public EditorWindow
{
public:
	RandomTest(const char* name = "Random Test", bool start_active = false);
	void Draw() override;

	int minInt = 0, maxInt = 10, resultInt = 0;
	float minF = 0.f, maxF = 1.f, resultF = 0.f;
};

class RendererTest : public EditorWindow
{
public:
	RendererTest(const char* name = "Render Test", bool start_active = true);
	void Draw() override;

	int shader_selcted, texture_selected, object_selected = 0;
};

enum GeoFigureType
{
	GEO_SPHERE = 0,
	GEO_CAPSULE,
	GEO_AABB,
	GEO_OBB,
	GEO_FRUSTUM,
	GEO_PLANE
	//GEO_TRINAGLE
};

class GeometryTest : public EditorWindow
{
public:
	GeometryTest(const char* name = "Geometry Test", bool start_active = false);
	void Draw() override;

private:
	float first_x, first_y, first_z = 0.f;
	float second_x, second_y, second_z = 0.f;
	float first_xtra, second_xtra = 0.f;
	int first_type, second_type = 0;
	bool intersects = false;
	std::string fig1 = "";
	std::string fig2 = "";
};

/*/ Missing windows:
void PlayPause();*/

#endif // !__MODULEEDITOR__