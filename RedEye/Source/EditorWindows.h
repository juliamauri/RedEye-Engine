#ifndef __EDITORWINDOWS__
#define __EDITORWINDOWS__

#include "MathGeoLib/include/Math/float4.h"
#include "ImGui\imgui.h"
#include "ImGuizmo/ImGuizmo.h"
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <EASTL/map.h>

class RE_GameObject;

class EditorWindow
{
public:
	EditorWindow(const char* name, bool start_active);
	virtual ~EditorWindow();

	void DrawWindow(bool secondary = false);

	bool IsActive() const;
	void SwitchActive();

	const char* Name() const;

private:

	virtual void Draw(bool secondary = false) = 0;

protected:

	bool active, lock_pos;
	ImVec2 pos, size, anchor;
	const char* name = nullptr;
};

class ConsoleWindow : public EditorWindow
{
public:
	ConsoleWindow(const char* name = "Console", bool start_active = true);
	~ConsoleWindow();

	void ChangeFilter(const int new_filter);
	void SwapCategory(const unsigned int category);

	void AppendLog(unsigned int category, const char* text, const char* file_name);

private:

	void Draw(bool secondary = false) override;
	void ResetBuffer();

private:

	ImGuiTextBuffer console_buffer;
	bool needs_rewriting = false;
	bool scroll_to_bot = true;
	int file_filter = -1;
	bool categories[8] = {};

	struct RE_Log
	{
		unsigned int caller_id, category, count;
		eastl::string data;
	};

	eastl::vector<RE_Log> logHistory;
	eastl::map<eastl::string, unsigned int> callers;
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


class AboutWindow : public EditorWindow
{
public:
	AboutWindow(const char* name = "About", bool start_active = false);
	~AboutWindow();

private:

	void Draw(bool secondary = false) override;

public:

	struct SoftwareInfo { const char *name, *version, *website; };
	eastl::vector<SoftwareInfo> sw_info;
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

	unsigned long long first = 0, max = 0, min = 0, count = 0;
	bool generating = false;
	int loops[2] = { 100, 100 };
};

class PlayPauseWindow : public EditorWindow
{
public:
	PlayPauseWindow(const char* name = "Play Controls", bool start_active = true);
	~PlayPauseWindow();

private:

	void Draw(bool secondary = false) override;
};

class PopUpWindow :public EditorWindow
{
public:
	PopUpWindow(const char* name = "PopUp", bool start_active = false);
	~PopUpWindow();

	void PopUp(const char* btnText = "Accept", const char* title = "PopUp", bool disableAllWindows = false);

	void PopUpError();
	void PopUpSave(bool fromExit = false, bool newScene = false);
	void PopUpPrefab(RE_GameObject* go);
	void PopUpDelRes(const char* res);
	void PopUpDelUndeFile(const char* assetPath);

	void AppendScopedLog(const char* log, unsigned int type);
	void ClearScope();

private:

	void Draw(bool secondary = false) override;

public:

	eastl::string logs, errors, warnings, solutions;

private:

	enum PU_STATE {
		PU_NONE = -1,
		PU_ERROR,
		PU_SAVE,
		PU_PREFAB,
		PU_DELETERESOURCE,
		PU_DELETEUNDEFINEDFILE
	} state = PU_NONE;
	bool exitAfter = false;
	bool inputName = false;
	bool spawnNewScene = false;
	eastl::string btnText;
	eastl::string titleText;
	eastl::string nameStr;
	RE_GameObject* goPrefab = nullptr;
	const char* resourceToDelete = nullptr;
	eastl::vector<const char*> resourcesUsing;
	bool resourceOnScene = false;
};

class AssetsWindow : public EditorWindow
{
public:
	AssetsWindow(const char* name = "Assets", bool start_active = true);
	~AssetsWindow();

	const char* GetCurrentDirPath() const;
	void SelectUndefined(eastl::string* toFill);

private:
	void Draw(bool secondary = false) override;

	const char* currentPath = nullptr;
	eastl::string* selectingUndefFile = nullptr;
};

class WwiseWindow : public EditorWindow
{
public:
	WwiseWindow(const char* name = "Wwise", bool start_active = true);
	~WwiseWindow();

private:
	void Draw(bool secondary = false) override;
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
	eastl::string matName, assetPath;
};

class RE_SkyBox;
class SkyBoxEditorWindow :public EditorWindow
{
public:
	SkyBoxEditorWindow(const char* name = "Skybox Editor", bool start_active = false);
	~SkyBoxEditorWindow();

private:
	void Draw(bool secondary = false) override;

	RE_SkyBox* editingSkybox = nullptr;
	eastl::string sbName, assetPath;

	unsigned int previewImage = 0;
};

class RE_Shader;
class ShaderEditorWindow :public EditorWindow
{
public:
	ShaderEditorWindow(const char* name = "Shader Editor", bool start_active = false);
	~ShaderEditorWindow();

private:
	void Draw(bool secondary = false) override;

	RE_Shader* editingShader = nullptr;
	eastl::string shaderName, assetPath;
	eastl::string vertexPath, fragmentPath, geometryPath;
};

class WaterPlaneResourceWindow : public EditorWindow
{
public: 
	WaterPlaneResourceWindow(const char* name = "Water As Resource", bool start_active = false);
	~WaterPlaneResourceWindow();

private:
	void Draw(bool secondary = false) override;

	eastl::string waterResouceName;
	bool deferred = false;
};

class TextEditor;
class RE_FileBuffer;
class TextEditorManagerWindow :public EditorWindow
{
public:
	TextEditorManagerWindow(const char* name = "TExt Editor Manager", bool start_active = false);
	~TextEditorManagerWindow();

	void PushEditor(const char* filePath, eastl::string* newFile = nullptr, const char* shadertTemplate = nullptr, bool* open = nullptr);

private:
	void Draw(bool secondary = false) override;

	struct editor {
		eastl::string* toModify = nullptr;
		TextEditor* textEditor = nullptr;
		RE_FileBuffer* file = nullptr;
		bool save = false;
		bool* open = nullptr;
		bool compiled = false;
		bool works = false;
	};

	eastl::vector<editor*> editors;
};

class SceneEditorWindow :public EditorWindow
{
public:
	SceneEditorWindow(const char* name = "Editor Scene", bool start_active = true);
	~SceneEditorWindow();

	unsigned int GetSceneWidht()const { return (width == 0) ? 500 : width; }
	unsigned int GetSceneHeight()const { return (heigth == 0) ? 500 : heigth; }

	bool isSelected()const { return isWindowSelected; }

	void UpdateViewPort();
	void Recalc();

	ImGuizmo::OPERATION GetOperation() const { return operation; }
	ImGuizmo::MODE GetMode() const{ return mode; }

	void SetOperation(ImGuizmo::OPERATION o) { operation = o; }
	void SetMode(ImGuizmo::MODE m) { mode = m; }

private:
	void Draw(bool secondary = false) override;

	math::float4 viewport = math::float4::zero;
	int width = 0;
	int heigth = 0;

	ImGuizmo::OPERATION operation = ImGuizmo::OPERATION::TRANSLATE;
	ImGuizmo::MODE mode = ImGuizmo::MODE::LOCAL;

	bool isWindowSelected = false;
	bool recalc = false;
};

class SceneGameWindow :public EditorWindow
{
public:
	SceneGameWindow(const char* name = "Game Scene", bool start_active = true);
	~SceneGameWindow();

	unsigned int GetSceneWidht()const { return (width == 0) ? 500 : width; }
	unsigned int GetSceneHeight()const { return (heigth == 0) ? 500 : heigth; }

	bool isSelected()const { return isWindowSelected; }

	void UpdateViewPort();
	void Recalc();

private:
	void Draw(bool secondary = false) override;

	math::float4 viewport = math::float4::zero;
	int width = 0;
	int heigth = 0;

	bool isWindowSelected = false;
	bool recalc = false;
};

class TransformDebugWindow : public EditorWindow
{
public:
	TransformDebugWindow(const char* name = "Transforms Debug Info", bool start_active = true);
	~TransformDebugWindow();

private:
	void Draw(bool secondary = false) override;
};

class RendererDebugWindow : public EditorWindow
{
public:
	RendererDebugWindow(const char* name = "Renderer Debug Info", bool start_active = true);
	~RendererDebugWindow();

private:
	void Draw(bool secondary = false) override;
};

#endif // !__EDITORWINDOWS__