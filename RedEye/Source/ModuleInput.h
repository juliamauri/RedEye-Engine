#ifndef __MODULEINPUT_H__
#define __MODULEINPUT_H__

#include "Module.h"
#include "Event.h"
#include <queue>

#define MAX_MOUSE_BUTTONS 5

enum KEY_STATE
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

	bool AddEvent(const Event e);

	KEY_STATE GetKey(int id) const;
	const MouseData* GetMouse() const;

private:

	void UpdateKeyboard();
	void HandleEventQueue();

private:

	KEY_STATE* keyboard;
	MouseData mouse;
	std::queue<Event> re_events;
};

#endif // !__MODULEINPUT_H__