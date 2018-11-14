#include "Resource.h"

#include "Globals.h"
#include "RE_Mesh.h"

ResourceContainer::ResourceContainer(const char* _name, const char * _origin, Resource_Type type, const char* md5)
{
	origin = (_origin != nullptr) ? _origin : "from Scratch";
	name = (_name != nullptr) ? _name : "unnkown";
	this->type = type;
	if(md5)
		this->md5 = new std::string(md5);
}

ResourceContainer::~ResourceContainer()
{
	/*
	if(name)
		DEL(name);
	if(origin)
		DEL(origin);
		*/
	if(md5)
		DEL(md5);
}

const char * ResourceContainer::GetName() const
{
	return name;
}

const char * ResourceContainer::GetOrigin() const
{
	return origin;
}

std::string* ResourceContainer::GetMD5() const
{
	return md5;
}

Resource_Type ResourceContainer::GetType() const
{
	return type;
}

void ResourceContainer::SetType(Resource_Type type)
{
	this->type = type;
}

void ResourceContainer::SetMD5(const char * md5)
{
	if (this->md5)
		DEL(this->md5);

	this->md5 = new std::string(md5);
}
