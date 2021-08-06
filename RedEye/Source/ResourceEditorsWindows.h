#ifndef __RESOURCE_EDITOR_WINDOWS__
#define __RESOURCE_EDITOR_WINDOWS__

#include "Event.h"
#include "EditorWindow.h"

#include <MGL/Math/float4.h>
#include "ImGui\imgui.h"
#include "ImGuiWidgets/ImGuizmo/ImGuizmo.h"

#include <EASTL/string.h>
#include <EASTL/vector.h>

class MaterialEditorWindow :public EditorWindow
{
public:
	MaterialEditorWindow(const char* name = "Material Editor", bool start_active = false);
	~MaterialEditorWindow();

private:
	void Draw(bool secondary = false) override;

	class RE_Material* editingMaerial = nullptr;
	eastl::string matName, assetPath;
};

class SkyBoxEditorWindow :public EditorWindow
{
public:
	SkyBoxEditorWindow(const char* name = "Skybox Editor", bool start_active = false);
	~SkyBoxEditorWindow();

private:
	void Draw(bool secondary = false) override;

	class RE_SkyBox* editingSkybox = nullptr;
	eastl::string sbName, assetPath;

	unsigned int previewImage = 0;
};

class ShaderEditorWindow :public EditorWindow
{
public:
	ShaderEditorWindow(const char* name = "Shader Editor", bool start_active = false);
	~ShaderEditorWindow();

private:
	void Draw(bool secondary = false) override;

	class RE_Shader* editingShader = nullptr;
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

class RE_ParticleEmitter;
class RE_ParticleEmitterBase;

class ParticleEmitterEditorWindow : public EditorWindow
{
public:
	ParticleEmitterEditorWindow(const char* name = "Particle Emitter Workspace", bool start_active = false);
	~ParticleEmitterEditorWindow();

	void StartEditing(RE_ParticleEmitter* sim, const char* md5);
	const RE_ParticleEmitter* GetEdittingParticleEmitter()const { return simulation; }

	void SaveEmitter(bool close = false, const char* emitter_name = nullptr, const char* emissor_base = nullptr, const char* renderer_base = nullptr);
	void NextOrClose();
	void CloseEditor();
	void LoadNextEmitter();

	unsigned int GetSceneWidht()const { return (width == 0) ? 500 : width; }
	unsigned int GetSceneHeight()const { return (heigth == 0) ? 500 : heigth; }

	bool isSelected()const { return isWindowSelected; }

	void UpdateViewPort();
	void Recalc();

private:
	void Draw(bool secondary = false) override;


private:
	const char* emiter_md5 = nullptr;
	RE_ParticleEmitter* simulation = nullptr;
	RE_ParticleEmitterBase* new_emitter = nullptr;

	bool load_next = false;
	const char* next_emiter_md5 = nullptr;
	RE_ParticleEmitter* next_simulation = nullptr;

	math::float4 viewport = math::float4::zero;
	int width = 0;
	int heigth = 0;

	bool isWindowSelected = false;
	bool recalc = false;

	bool docking = false;
	bool need_save = false;
};

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
		class TextEditor* textEditor = nullptr;
		class RE_FileBuffer* file = nullptr;
		bool save = false;
		bool* open = nullptr;
		bool compiled = false;
		bool works = false;
	};

	eastl::vector<editor*> editors;
};

#endif // !__RESOURCE_EDITOR_WINDOWS__