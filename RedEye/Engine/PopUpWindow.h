#ifndef __POP_UP_WINDOW__
#define __POP_UP_WINDOW__

#include "EditorWindow.h"

class PopUpWindow :public EditorWindow
{
public:
	PopUpWindow(const char* name = "PopUp", bool start_active = false) : EditorWindow(name, start_active) {}
	~PopUpWindow() {}

	void PopUp(const char* btnText = "Accept", const char* title = "PopUp", bool disableAllWindows = false);

	void PopUpError();
	void PopUpSaveScene(bool fromExit = false, bool newScene = false);
	void PopUpSaveParticles(bool need_particle_names = false, bool not_name = false, bool not_emissor = false, bool not_renderer = false, bool close_after = false);
	void PopUpPrefab(class RE_GameObject* go);
	void PopUpDelRes(const char* res);
	void PopUpDelUndeFile(const char* assetPath);

	void AppendScopedLog(const char* log, RE_EventType type);
	void ClearScope();

private:

	void Draw(bool secondary = false) override;

public:

	eastl::string logs, errors, warnings, solutions;

private:

	enum class PopUpState : signed char
	{
		NONE = -1,
		ERROR,
		SAVE_SCENE,
		SAVE_PARTICLE_EMITTER,
		PREFAB,
		DELETE_RESOURCE,
		DELETE_UNDEFINED_FILE
	} state = PopUpState::NONE;

	unsigned char flags = 0;
	eastl::string btn_text, title_text, name_str, emission_name, renderer_name;
	RE_GameObject* go_prefab = nullptr;
	const char* resource_to_delete = nullptr;
	eastl::vector<const char*> using_resources;
};

#endif // !__POP_UP_WINDOW__