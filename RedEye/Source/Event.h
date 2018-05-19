#ifndef __EVENT_H__
#define __EVENT_H__

class EventListener;
class SDL_Event;

typedef enum RE_EventType : unsigned short int
{
	SDL_EVENT = 0x00,

	CONFIG_LOADED,
	PLAY,
	PAUSE,
	UNPAUSE,
	STOP,

	REQUEST_SAVE,
	REQUEST_LOAD,

	MAX_EVENT_TYPES
};

struct RE_Event
{
	//RE_EventType type;
	unsigned int timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
	// ++Event data

	void CheckTime();
};

union EventData
{
	RE_Event	re_event;
	SDL_Event*	sdl_event;
};

class Event
{
public:

	Event(EventListener* lis, RE_EventType t);
	Event(EventListener* lis, SDL_Event* e);
	~Event();

	void CallListener();
	bool ValidEvent();

private:

	void SetInvalid();

private:

	EventData data;
	RE_EventType type = MAX_EVENT_TYPES;
	EventListener* listener = nullptr;
};

#endif

// int SDL_EventFilter(void* userdata, SDL_Event* event)