#ifndef __EVENT_H__
#define __EVENT_H__

#include "Cvar.h"

class EventListener;

enum RE_EventType : unsigned short int
{
	// APP
	PLAY,
	PAUSE,
	TICK,
	STOP,

	REQUEST_LOAD,
	REQUEST_SAVE,
	REQUEST_QUIT,

	// Window
	WINDOW_MOVED,
	WINDOW_SIZE_CHANGED,

	// Scene
	GO_CHANGED_TO_ACTIVE,
	GO_CHANGED_TO_INACTIVE,
	GO_CHANGED_TO_STATIC,
	GO_CHANGED_TO_NON_STATIC,
	GO_HAS_NEW_CHILD,
	DESTROY_GO,
	TRANSFORM_MODIFIED,
	PLANE_CHANGE_TO_MESH,

	// Renderer
	SET_VSYNC,
	SET_DEPTH_TEST,
	SET_FACE_CULLING,
	SET_LIGHTNING,
	SET_TEXTURE_TWO_D,
	SET_COLOR_MATERIAL,
	SET_WIRE_FRAME,
	EDITORWINDOWCHANGED,
	GAMEWINDOWCHANGED,

	//Editor
	UPDATE_SCENE_WINDOWS,
	EDITOR_SCENE_RAYCAST,

	SCOPE_PROCEDURE_START,
	SCOPE_PROCEDURE_END,

	CONSOLE_LOG_MIN,
	CONSOLE_LOG_SEPARATOR,
	CONSOLE_LOG_GLOBAL,
	CONSOLE_LOG_SECONDARY,
	CONSOLE_LOG_TERCIARY,
	CONSOLE_LOG_SOFTWARE,
	CONSOLE_LOG_ERROR,
	CONSOLE_LOG_WARNING,
	CONSOLE_LOG_SOLUTION,
	CONSOLE_LOG_SAVE_ERROR,
	CONSOLE_LOG_SAVE_WARNING,
	CONSOLE_LOG_SAVE_SOLUTION,
	CONSOLE_LOG_MAX,

	// Resources
	RESOURCE_CHANGED,

	MAX_EVENT_TYPES
};

class Event
{
public:

	Event(RE_EventType t, EventListener* lis, Cvar data = Cvar(), Cvar data2 = Cvar());
	Event(Event& e);
	~Event() {}

	static void Push(RE_EventType t, EventListener* lis, Cvar data = Cvar(), Cvar data2 = Cvar());
	static void PushForced(RE_EventType t, EventListener* lis, Cvar data = Cvar(), Cvar data2 = Cvar());
	static void PumpAll();
	static void ClearQueue();
	static void ResumeEvents();
	static void PauseEvents();
	static bool isPaused();

private:

	void CallListener() const;
	bool IsValid() const;

public:

	RE_EventType type;
	EventListener* listener;
	Cvar data1;
	Cvar data2;
	const unsigned int timestamp;

private:

	static bool paused;
};

#endif