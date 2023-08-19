#include "Event.h"

#include "ModuleInput.h"

#include "RE_Memory.h"
#include "RE_Profiler.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleScene.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"
#include "RE_FileSystem.h"
#include "PopUpWindow.h"

#include <SDL2/SDL.h>

// Called before render is available
bool ModuleInput::Init()
{
	RE_PROFILE(RE_ProfiledFunc::Init, RE_ProfiledClass::ModuleInput);
	RE_LOG("Initializing Module Input");
	RE_LOG_SECONDARY("Init SDL input & event subsystems");
	bool ret = true;

	if(SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
	{
		RE_LOG_ERROR("SDL_EVENTS could not initialize! SDL_Error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

// Called every draw update
void ModuleInput::PreUpdate()
{
	RE_PROFILE(RE_ProfiledFunc::PreUpdate, RE_ProfiledClass::ModuleInput);

	SDL_PumpEvents();

	// Mouse
	mouse.UpdateButtons();
	mouse.ResetMotion();

	// Keyboard
	if (!ImGui::GetIO().WantCaptureKeyboard)
	{
		const Uint8* keys = SDL_GetKeyboardState(NULL);
		for (int i = 0; i < MAX_KEYS; ++i)
		{
			if (keys[i] == 1)
			{
				if (keyboard[i] == KEY_STATE::KEY_IDLE) keyboard[i] = KEY_STATE::KEY_DOWN;
				else keyboard[i] = KEY_STATE::KEY_REPEAT;
			}
			else
			{
				if (keyboard[i] == KEY_STATE::KEY_REPEAT || keyboard[i] == KEY_STATE::KEY_DOWN) keyboard[i] = KEY_STATE::KEY_UP;
				else keyboard[i] = KEY_STATE::KEY_IDLE;
			}
		}
	}

	// Events SDL (and send to Editor's ImGui)
	HandleSDLEventQueue();

	// Call all own events' listeners
	while (!events_queue.empty())
	{
		const Event e = events_queue.front();
		if (e.type < RE_EventType::MAX_EVENT_TYPES && e.listener != nullptr)
			e.listener->RecieveEvent(e);
		events_queue.pop();
	}
}

void ModuleInput::DrawEditor()
{
	ImGui::Text("Mouse");
	ImGui::Text("X: %u\tY: %u", mouse.mouse_x, mouse.mouse_y);
	ImGui::Text("MotionX: %i\tMotionY: %i", mouse.mouse_x_motion, mouse.mouse_y_motion);
	ImGui::Text("Wheel Motion: %i", mouse.mouse_wheel_motion);
}

// Called before quitting
void ModuleInput::CleanUp()
{
	RE_PROFILE(RE_ProfiledFunc::CleanUp, RE_ProfiledClass::ModuleInput);
	SDL_QuitSubSystem(SDL_INIT_EVENTS);
	while (!events_queue.empty()) events_queue.pop();
}

KEY_STATE ModuleInput::GetKey(const unsigned int id) const
{
	RE_ASSERT(id < MAX_KEYS);
	return keyboard[id];
}

bool ModuleInput::CheckKey(const unsigned int id, const KEY_STATE state) const
{
	RE_ASSERT(id < MAX_KEYS);
	return keyboard[id] == state;
}

const MouseData& ModuleInput::GetMouse() const { return mouse; }

void ModuleInput::SetMouseAtCenter()
{
	// TODO Rub: Differenciate between whole window and views
	SDL_WarpMouseInWindow(RE_WINDOW->GetWindow(), RE_WINDOW->GetWidth() / 2, RE_WINDOW->GetHeight() / 2);
}

void ModuleInput::Push(RE_EventType t, EventListener* lis, RE_Cvar d1, RE_Cvar d2)
{
	if (!events_paused) events_queue.push(Event(t, lis, d1, d2));
}

void ModuleInput::PushForced(RE_EventType t, EventListener* lis, RE_Cvar d1, RE_Cvar d2)
{
	events_queue.push(Event(t, lis, d1, d2));
}

void ModuleInput::HandleSDLEventQueue()
{
	// SDL Events
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		RE_EDITOR->HandleSDLEvent(&e);

		switch (e.type)
		{
		/* Application events */
		case SDL_QUIT:/**< User-requested quit */
			if (RE_SCENE->HasChanges()) RE_EDITOR->popupWindow->PopUpSaveScene(true);
			else Push(RE_EventType::REQUEST_QUIT, App);
			break;
		case SDL_APP_TERMINATING:/**< The application is being terminated by the OS
								Called on iOS in applicationWillTerminate()
								Called on Android in onDestroy() */ break;
		case SDL_APP_LOWMEMORY:/**< The application is low on memory, free memory if possible.
								Called on iOS in applicationDidReceiveMemoryWarning()
								Called on Android in onLowMemory()*/ break;
		case SDL_APP_WILLENTERBACKGROUND:/**< The application is about to enter the background
								 Called on iOS in applicationWillResignActive()
								 Called on Android in onPause() */ break;
		case SDL_APP_DIDENTERBACKGROUND:/**< The application did enter the background and may not get CPU for some time
								Called on iOS in applicationDidEnterBackground()
								Called on Android in onPause() */ break;
		case SDL_APP_WILLENTERFOREGROUND:/**< The application is about to enter the foreground
								 Called on iOS in applicationWillEnterForeground()
								 Called on Android in onResume() */ break;
		case SDL_APP_DIDENTERFOREGROUND:/**< The application is now interactive
								Called on iOS in applicationDidBecomeActive()
								Called on Android in onResume() */ break;

		/* Window events */
		case SDL_WINDOWEVENT:/**< System specific event */
			switch (e.window.event)
			{
			case SDL_WINDOWEVENT_SHOWN:/**< Window has been shown */ break;
			case SDL_WINDOWEVENT_HIDDEN:/**< Window has been hidden */ break;
			case SDL_WINDOWEVENT_EXPOSED:/**< Window has been exposed and should be redrawn */ break;
			case SDL_WINDOWEVENT_MOVED:/**< Window has been moved to data1, data2 */
				Push(RE_EventType::WINDOW_MOVED, RE_WINDOW, RE_Cvar(e.window.data1), RE_Cvar(e.window.data2));
				break;
			case SDL_WINDOWEVENT_RESIZED:/**< Window has been resized to data1xdata2 */
				// this sneaky event is always preceded by SDL_WINDOWEVENT_SIZE_CHANGED
				//App->renderer3d->MainContextChanged(width = e->window.data1, height = e->window.data2);
				break;
			case SDL_WINDOWEVENT_SIZE_CHANGED:/**< The window size has changed, either as a result of an API call or through the system or user changing the window size. */
				Push(RE_EventType::WINDOW_SIZE_CHANGED, RE_WINDOW, RE_Cvar(e.window.data1), RE_Cvar(e.window.data2));
				break;
			case SDL_WINDOWEVENT_MINIMIZED:/**< Window has been minimized */ break;
			case SDL_WINDOWEVENT_MAXIMIZED:/**< Window has been maximized */ break;
			case SDL_WINDOWEVENT_RESTORED:/**< Window has been restored to normal size and position */ break;
			case SDL_WINDOWEVENT_ENTER:/**< Window has gained mouse focus */ break;
			case SDL_WINDOWEVENT_LEAVE:/**< Window has lost mouse focus */ break;
			case SDL_WINDOWEVENT_FOCUS_GAINED:/**< Window has gained keyboard focus */ break;
			case SDL_WINDOWEVENT_FOCUS_LOST:/**< Window has lost keyboard focus */ break;
			case SDL_WINDOWEVENT_CLOSE:/**< The window manager requests that the window be closed */
			{
				if (RE_SCENE->HasChanges()) RE_EDITOR->popupWindow->PopUpSaveScene(true);
				else Push(RE_EventType::REQUEST_QUIT, App);
				break;
			}
			}
			break;
		case SDL_SYSWMEVENT:/**< System specific event */ break;

		/* Keyboard events */
		case SDL_KEYDOWN:/**< Key pressed */ break;
		case SDL_KEYUP:/**< Key released */ break;
		case SDL_TEXTEDITING:/**< Keyboard text editing (composition) */ break;
		case SDL_TEXTINPUT:/**< Keyboard text input */ break;

		/* Mouse events */
		case SDL_MOUSEMOTION:/**< Mouse moved */
			if (!ImGui::GetIO().WantCaptureMouse)
			{
				mouse.mouse_x = e.motion.x;
				mouse.mouse_y = e.motion.y;
				mouse.mouse_x_motion = e.motion.xrel;
				mouse.mouse_y_motion = e.motion.yrel;
			}
			break;
		case SDL_MOUSEBUTTONDOWN: /**< Mouse button pressed */ break;
		case SDL_MOUSEBUTTONUP: /**< Mouse button released */ break;
		case SDL_MOUSEWHEEL: /**< Mouse wheel motion */
			if (!ImGui::GetIO().WantCaptureMouse)
			{
				mouse.mouse_wheel_motion = e.wheel.y;
			}
			break;

		/* Joystick events */
		case SDL_JOYAXISMOTION:/**< Joystick axis motion */ break;
		case SDL_JOYBALLMOTION:/**< Joystick trackball motion */ break;
		case SDL_JOYHATMOTION:/**< Joystick hat position change */ break;
		case SDL_JOYBUTTONDOWN:/**< Joystick button pressed */ break;
		case SDL_JOYBUTTONUP:/**< Joystick button released */ break;
		case SDL_JOYDEVICEADDED:/**< A new joystick has been inserted into the system */ break;
		case SDL_JOYDEVICEREMOVED:/**< An opened joystick has been removed */ break;

		/* Game controller events */
		case SDL_CONTROLLERAXISMOTION:/**< Game controller axis motion */ break;
		case SDL_CONTROLLERBUTTONDOWN:/**< Game controller button pressed */ break;
		case SDL_CONTROLLERBUTTONUP:/**< Game controller button released */ break;
		case SDL_CONTROLLERDEVICEADDED:/**< A new Game controller has been inserted into the system */ break;
		case SDL_CONTROLLERDEVICEREMOVED:/**< An opened Game controller has been removed */ break;
		case SDL_CONTROLLERDEVICEREMAPPED:/**< The controller mapping was updated */ break;

		/* Touch events */
		case SDL_FINGERDOWN: break;
		case SDL_FINGERUP: break;
		case SDL_FINGERMOTION: break;

		/* Gesture events */
		case SDL_DOLLARGESTURE: break;
		case SDL_DOLLARRECORD: break;
		case SDL_MULTIGESTURE: break;

		/* Clipboard events */
		case SDL_CLIPBOARDUPDATE:/**< The clipboard changed */ break;

		/* Drag and drop events */
		case SDL_DROPFILE:/**< The system requests a file open */
			RE_LOG("File Dropped: %s", e.drop.file);
			RE_FS->HandleDropedFile(e.drop.file);
			break;

		/* Render events */
		case SDL_RENDER_TARGETS_RESET:/**< The render targets have been reset and their contents need to be updated */ break;
		case SDL_RENDER_DEVICE_RESET:/**< The device has been reset and all textures need to be recreated */ break;
		}
	}
}

// --------------------- SDL Event Types ---------------------
//SDL_CommonEvent common;         /**< Common event data */
//SDL_WindowEvent window;         /**< Window event data */
//SDL_KeyboardEvent key;          /**< Keyboard event data */
//SDL_TextEditingEvent edit;      /**< Text editing event data */
//SDL_TextInputEvent text;        /**< Text input event data */
//SDL_MouseMotionEvent motion;    /**< Mouse motion event data */
//SDL_MouseButtonEvent button;    /**< Mouse button event data */
//SDL_MouseWheelEvent wheel;      /**< Mouse wheel event data */
//SDL_JoyAxisEvent jaxis;         /**< Joystick axis event data */
//SDL_JoyBallEvent jball;         /**< Joystick ball event data */
//SDL_JoyHatEvent jhat;           /**< Joystick hat event data */
//SDL_JoyButtonEvent jbutton;     /**< Joystick button event data */
//SDL_JoyDeviceEvent jdevice;     /**< Joystick device change event data */
//SDL_ControllerAxisEvent caxis;      /**< Game Controller axis event data */
//SDL_ControllerButtonEvent cbutton;  /**< Game Controller button event data */
//SDL_ControllerDeviceEvent cdevice;  /**< Game Controller device event data */
//SDL_AudioDeviceEvent adevice;   /**< Audio device event data */
//SDL_QuitEvent quit;             /**< Quit request event data */
//SDL_UserEvent user;             /**< Custom event data */
//SDL_SysWMEvent syswm;           /**< System dependent window event data */
//SDL_TouchFingerEvent tfinger;   /**< Touch finger event data */
//SDL_MultiGestureEvent mgesture; /**< Gesture event data */
//SDL_DollarGestureEvent dgesture; /**< Gesture event data */
//SDL_DropEvent drop;             /**< Drag and drop event data */

void MouseData::ResetMotion() { mouse_x_motion = mouse_y_motion = mouse_wheel_motion = 0; }

void MouseData::UpdateButtons()
{
	Uint32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

	for (int i = 0; i < 5; ++i)
	{
		if (buttons & SDL_BUTTON(i))
		{
			if (mouse_buttons[i] == KEY_STATE::KEY_IDLE) mouse_buttons[i] = KEY_STATE::KEY_DOWN;
			else mouse_buttons[i] = KEY_STATE::KEY_REPEAT;
		}
		else
		{
			if (mouse_buttons[i] == KEY_STATE::KEY_REPEAT || mouse_buttons[i] == KEY_STATE::KEY_DOWN) mouse_buttons[i] = KEY_STATE::KEY_UP;
			else mouse_buttons[i] = KEY_STATE::KEY_IDLE;
		}
	}
}

KEY_STATE MouseData::GetButton(int id) const { return mouse_buttons[id]; }
bool MouseData::Moved() const { return (mouse_x_motion != 0 || mouse_y_motion != 0); }
