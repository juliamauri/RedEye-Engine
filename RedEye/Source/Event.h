#ifndef __EVENT_H__
#define __EVENT_H__

class EventListener;

typedef enum : unsigned short int
{
	//SDL_EVENT = 0x00,

	CONFIG_LOADED,
	PLAY,
	PAUSE,
	UNPAUSE,
	STOP,

	REQUEST_SAVE,
	REQUEST_LOAD,
	REQUEST_QUIT,

	MAX_EVENT_TYPES
} RE_EventType;

struct RE_Event
{
	unsigned int timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
	// ++Event data

	void CheckTime();
};

class Event
{
public:

	Event();
	Event(RE_EventType t, EventListener* lis = nullptr);
	Event(const Event& e);
	~Event();

	void CallListener() const;
	bool IsValid() const;

private:

	void SetInvalid();

public:

	RE_Event data;
	RE_EventType type;

private:

	EventListener* listener;
};

#endif

// int SDL_EventFilter(void* userdata, SDL_Event* event)