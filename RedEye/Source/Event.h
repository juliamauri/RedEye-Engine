#ifndef __EVENT_H__
#define __EVENT_H__

#include "Cvar.h"
#include <queue>

class EventListener;

typedef enum : unsigned short int
{
	// APP
	PLAY,
	PAUSE,
	UNPAUSE,
	TICK,
	STOP,

	REQUEST_DEFAULT_CONF,
	REQUEST_LOAD,
	REQUEST_SAVE,
	REQUEST_QUIT,

	// Window
	WINDOW_MOVED,
	WINDOW_SIZE_CHANGED,

	// Scene
	TRANSFORM_MODIFIED,
	STATIC_TRANSFORM_MODIFIED,

	// Renderer
	SET_VSYNC,
	SET_DEPTH_TEST,
	SET_FACE_CULLING,
	SET_LIGHTNING,
	SET_TEXTURE_TWO_D,
	SET_COLOR_MATERIAL,
	SET_WIRE_FRAME,
	CURRENT_CAM_VIEWPORT_CHANGED,

	// Gameobject
	PARENT_TRANSFORM_MODIFIED,

	MAX_EVENT_TYPES
} RE_EventType;

class Event
{
public:

	Event(RE_EventType t, EventListener* lis = nullptr, Cvar data = Cvar(), Cvar data2 = Cvar());
	Event(const Event& e);
	~Event();

	void CallListener() const;
	bool IsValid() const;

	unsigned int GetTimeStamp() const;
	RE_EventType GetType() const;

	const Cvar& GetData() const;
	const Cvar& GetDataNext() const;

	static void Push(RE_EventType t, EventListener* lis = nullptr, Cvar data = Cvar(), Cvar data2 = Cvar());
	static void PumpAll();

private:

	void Clear();

private:

	RE_EventType type;
	EventListener* listener;
	const Cvar data1;
	const Cvar data2;
	const unsigned int timestamp;

	static std::queue<Event> events_queue;
};

#endif

// int SDL_EventFilter(void* userdata, SDL_Event* event)