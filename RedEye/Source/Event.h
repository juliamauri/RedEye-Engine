#ifndef __EVENT_H__
#define __EVENT_H__

#include "Cvar.h"
#include <queue>

class EventListener;

typedef enum RE_EventType : unsigned short int
{
	// APP
	PLAY,
	PAUSE,
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
	GO_CHANGED_TO_ACTIVE,
	GO_CHANGED_TO_INACTIVE,
	GO_CHANGED_TO_STATIC,
	GO_CHANGED_TO_NON_STATIC,
	GO_HAS_NEW_CHILD,
	GO_HAS_NEW_CHILDS,
	GO_REMOVE_CHILD,
	TRANSFORM_MODIFIED,

	// Renderer
	SET_VSYNC,
	SET_DEPTH_TEST,
	SET_FACE_CULLING,
	SET_LIGHTNING,
	SET_TEXTURE_TWO_D,
	SET_COLOR_MATERIAL,
	SET_WIRE_FRAME,
	CURRENT_CAM_VIEWPORT_CHANGED,

	MAX_EVENT_TYPES
};

class Event
{
public:

	Event(RE_EventType t, EventListener* lis, Cvar data = Cvar(), Cvar data2 = Cvar());
	Event(Event& e);
	virtual ~Event();

	static void Push(RE_EventType t, EventListener* lis, Cvar data = Cvar(), Cvar data2 = Cvar());
	static void PumpAll();

protected:

	void CallListener() const;
	bool IsValid() const;
	void Clear();

public:

	RE_EventType type;
	EventListener* listener;
	Cvar data1;
	Cvar data2;
	const unsigned int timestamp;

private:

	static std::queue<Event> events_queue;
};

class InstantEvent : public Event
{
public:

	InstantEvent(RE_EventType t, EventListener* lis, Cvar data = Cvar(), Cvar data2 = Cvar());
	InstantEvent(InstantEvent& e);
	~InstantEvent();
};

#endif

// int SDL_EventFilter(void* userdata, SDL_Event* event)