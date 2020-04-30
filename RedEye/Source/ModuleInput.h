#ifndef __MODULEINPUT_H__
#define __MODULEINPUT_H__

#include "Module.h"
#include "Event.h"

#define MAX_MOUSE_BUTTONS 5

enum KEY_STATE : short unsigned int
{
	KEY_IDLE = 0,
	KEY_DOWN,
	KEY_REPEAT,
	KEY_UP
};

struct MouseData
{
	KEY_STATE mouse_buttons[MAX_MOUSE_BUTTONS];

	int mouse_x;
	int mouse_y;
	int mouse_x_motion;
	int mouse_y_motion;
	int mouse_wheel_motion;

	void ResetMotion();
	void UpdateButtons();

	KEY_STATE GetButton(int id) const;
	bool Moved() const;
};

class ModuleInput : public Module
{
public:
	
	ModuleInput(const char* name, bool start_enabled = true);
	~ModuleInput();

	bool Init(JSONNode* config_module) override;
	update_status PreUpdate() override;
	void DrawEditor() override;
	bool CleanUp() override;

	KEY_STATE GetKey(const unsigned int id) const;
	bool CheckKey(const unsigned int id, const KEY_STATE state = KEY_UP) const;
	const MouseData& GetMouse() const;

	void SetMouseAtCenter();

private:

	void UpdateKeyboard();
	void HandleSDLEventQueue();

private:

	KEY_STATE* keyboard;
	MouseData mouse;
};

#endif // !__MODULEINPUT_H__