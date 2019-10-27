#ifndef __EDITORWINDOWS__
#define __EDITORWINDOWS__

#include "Resource.h"

#include "ImGui\imgui.h"
#include <list>
#include <string>
#include <vector>

class EditorWindow
{
public:
	EditorWindow(const char* name, bool start_active);
	virtual ~EditorWindow();

	void DrawWindow(bool secondary = false);

	bool IsActive() const;
	void SwitchActive();

	const char* Name() const;

	// Set position/size for window
	/*ImGui::SetNextWindowPos(ImVec2(0, 738));
	ImGui::SetWindowSize(ImVec2(1230.0f, 220.0f));*/

private:

	virtual void Draw(bool secondary = false) = 0;

protected:

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
	
	void ChangeFilter(const int new_filter);
	void SwapCategory(const unsigned int category);

private:

	void Draw(bool secondary = false) override;

public:

	ImGuiTextBuffer console_buffer;
	bool scroll_to_bot = true;
	int file_filter = -1;
	bool categories[8] = { true, true, true, true, true, true, true, true };
	const char* category_names[8] = { "Separator", "Global", "Secondary", "Terciary", "Error" , "Warning", "Solution" , "Software" };
};

class ConfigWindow : public EditorWindow
{
public:
	ConfigWindow(const char* name = "Configuration", bool start_active = true);

private:

	void Draw(bool secondary = false) override;

public:

	bool changed_config;
};

class HeriarchyWindow : public EditorWindow
{
public:
	HeriarchyWindow(const char* name = "Heriarchy", bool start_active = true);

private:

	void Draw(bool secondary = false) override;
};

class PropertiesWindow : public EditorWindow
{
public:
	PropertiesWindow(const char* name = "Properties", bool start_active = true);

private:

	void Draw(bool secondary = false) override;
};

struct SoftwareInfo
{
	SoftwareInfo(const char * name, const char * version = nullptr, const char * website = nullptr);
	std::string name, version, website;
};

class AboutWindow : public EditorWindow
{
public:
	AboutWindow(const char* name = "About", bool start_active = false);

private:

	void Draw(bool secondary = false) override;

public:

	std::list<SoftwareInfo> sw_info;
};

class RandomTest : public EditorWindow
{
public:
	RandomTest(const char* name = "Random Test", bool start_active = false);

private:

	void Draw(bool secondary = false) override;

public:

	int minInt = 0, maxInt = 10, resultInt = 0;
	float minF = 0.f, maxF = 1.f, resultF = 0.f;
};

class TexturesWindow : public EditorWindow
{
public:
	TexturesWindow(const char* name = "Texture Manager", bool start_active = false);

private:

	void Draw(bool secondary = false) override;
};

class EditorSettingsWindow : public EditorWindow
{
public:
	EditorSettingsWindow(const char* name = "Editor Settings", bool start_active = true);

private:

	void Draw(bool secondary = false) override;
};

class PlayPauseWindow : public EditorWindow
{
public:
	PlayPauseWindow(const char* name = "Play Controls", bool start_active = true);

private:

	void Draw(bool secondary = false) override;
};

class SelectFile : public EditorWindow
{
public:
	SelectFile(const char* name = "Select File", bool start_active = false);

	void Start(const char* path);

	std::string IsSelected();

private:
	void Draw(bool secondary = false) override;

	std::string path;
	std::string selected;
	char **rc = nullptr;
};

class PrefabsPanel :public EditorWindow
{
public:
	PrefabsPanel(const char* name = "Prefabs", bool start_active = false);

private:
	void Draw(bool secondary = false) override;

	std::vector<ResourceContainer*> prefabs;

	ResourceContainer* selected = nullptr;
};

class PopUpWindow :public EditorWindow
{
public:
	PopUpWindow(const char* name = "PopUp", bool start_active = false);

	void PopUp(const char* btnText = "Accept", const char* title = "PopUp", bool disableAllWindows = false);

	void PopUpError();

private:
	void Draw(bool secondary = false) override;

	bool disableAllWindows = false;
	bool fromHandleError = false;
	std::string btnText;
	std::string titleText;

};


#endif // !__EDITORWINDOWS__