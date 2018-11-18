#pragma once

#include "Event.h"

class EventListener
{
public:
	virtual void RecieveEvent(const Event& e) {}
};