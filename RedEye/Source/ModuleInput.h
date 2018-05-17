#ifndef __MODULEINPUT_H__
#define __MODULEINPUT_H__

#include "Module.h"
#include <queue>

#define MAX_MOUSE_BUTTONS 5

enum KEY_STATE
{
	KEY_IDLE = 0,
	KEY_DOWN,
	KEY_REPEAT,
	KEY_UP
};

class ModuleInput : public Module
{
public:
	
	ModuleInput(const char* name, bool start_enabled = true);
	~ModuleInput();

	bool Init();
	update_status PreUpdate(float dt);
	bool CleanUp();

	KEY_STATE GetKey(int id) const;
	KEY_STATE GetMouseButton(int id) const;
	int GetMouseX() const;
	int GetMouseY() const;
	int GetMouseXMotion() const;
	int GetMouseYMotion() const;
	int GetMouseWheelMotion() const;

	bool MouseMoved() const;

private:

	void UpdateKeyboard();
	void UpdateMouseButtons();
	void HandleEventQueue();

private:

	KEY_STATE* keyboard;
	KEY_STATE mouse_buttons[MAX_MOUSE_BUTTONS];
	int mouse_x;
	int mouse_y;
	int mouse_x_motion;
	int mouse_y_motion;
	int mouse_wheel_motion;
};

#endif // !__MODULEINPUT_H__