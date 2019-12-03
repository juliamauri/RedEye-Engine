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
	~ConsoleWindow();

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

class AssetsWindow : public EditorWindow
{
public:
	AssetsWindow(const char* name = "Assets Panel", bool start_active = true);
	~AssetsWindow();

	const char* GetCurrentDirPath()const;

private:
	void Draw(bool secondary = false) override;

	const char* currentPath = nullptr;
};

class ConfigWindow : public EditorWindow
{
public:
	ConfigWindow(const char* name = "Configuration", bool start_active = true);
	~ConfigWindow();

private:

	void Draw(bool secondary = false) override;

public:

	bool changed_config;
};

class HeriarchyWindow : public EditorWindow
{
public:
	HeriarchyWindow(const char* name = "Heriarchy", bool start_active = true);
	~HeriarchyWindow();

private:

	void Draw(bool secondary = false) override;
};

class PropertiesWindow : public EditorWindow
{
public:
	PropertiesWindow(const char* name = "Properties", bool start_active = true);
	~PropertiesWindow();

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
	~AboutWindow();

private:

	void Draw(bool secondary = false) override;

public:

	std::list<SoftwareInfo> sw_info;
};

class RandomTest : public EditorWindow
{
public:
	RandomTest(const char* name = "Random Test", bool start_active = false);
	~RandomTest();

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
	~TexturesWindow();

private:

	void Draw(bool secondary = false) override;
};

class PlayPauseWindow : public EditorWindow
{
public:
	PlayPauseWindow(const char* name = "Play Controls", bool start_active = true);
	~PlayPauseWindow();

private:

	void Draw(bool secondary = false) override;
};

class SelectFile : public EditorWindow
{
public:
	SelectFile(const char* name = "Select File", bool start_active = false);
	~SelectFile();

	void Start(const char* windowName, const char* path, std::string* forFill, bool selectFolder = false);

	void SelectTexture();

	ResourceContainer* GetSelectedTexture();

private:
	void Draw(bool secondary = false) override;

	void SendSelected();
	void Clear();

private:
	bool selectingFolder = false;

	std::string windowName;
	std::string path;
	std::string selected;
	char **rc = nullptr;
	char **selectedPointer = nullptr;

	std::string* toFill = nullptr;

	ResourceContainer* selectedTexture = nullptr;
	std::vector<ResourceContainer*> textures;
};

class PrefabsPanel :public EditorWindow
{
public:
	PrefabsPanel(const char* name = "Prefabs", bool start_active = false);
	~PrefabsPanel();

private:
	void Draw(bool secondary = false) override;

	std::vector<ResourceContainer*> prefabs;

	ResourceContainer* selected = nullptr;
};

class PopUpWindow :public EditorWindow
{
public:
	PopUpWindow(const char* name = "PopUp", bool start_active = false);
	~PopUpWindow();

	void PopUp(const char* btnText = "Accept", const char* title = "PopUp", bool disableAllWindows = false);

	void PopUpError();

private:
	void Draw(bool secondary = false) override;

	bool disableAllWindows = false;
	bool fromHandleError = false;
	std::string btnText;
	std::string titleText;

};

class RE_Material;
class MaterialEditorWindow :public EditorWindow
{
public:
	MaterialEditorWindow(const char* name = "Material Editor", bool start_active = false);
	~MaterialEditorWindow();

private:
	void Draw(bool secondary = false) override;

	RE_Material* editingMaerial = nullptr;
	std::string matName;
	std::string assetPath;
};

#endif // !__EDITORWINDOWS__