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

class Event
{
public:

	Event(RE_EventType t, EventListener* lis = nullptr);
	Event(RE_EventType t, unsigned int ts, EventListener* lis = nullptr);
	Event(const Event& e);
	~Event();

	void CallListener() const;
	bool IsValid() const;

	unsigned int GetTimeStamp() const;
	RE_EventType GetType() const;

private:

	void Clear();

private:

	RE_EventType type;
	EventListener* listener;
	unsigned int timestamp;
};

#endif

// int SDL_EventFilter(void* userdata, SDL_Event* event)