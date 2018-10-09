#include "Resource.h"

#include "Globals.h"

ResourceContainer::ResourceContainer(const char* _name, const char * _origin)
{
	origin = (_origin != nullptr) ? _origin : "from Scratch";
	name = (_name != nullptr) ? _name : "unnkown";
}

ResourceContainer::~ResourceContainer()
{
	DEL(name);
	DEL(origin);
}

const char * ResourceContainer::GetName() const
{
	return name;
}

const char * ResourceContainer::GetOrigin() const
{
	return origin;
}