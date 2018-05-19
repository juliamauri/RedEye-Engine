#pragma once

class Event;

class EventListener
{
public:
	virtual void RecieveEvent(const Event* e) {}
};