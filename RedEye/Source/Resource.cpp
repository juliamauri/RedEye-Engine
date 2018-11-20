#include "Resource.h"

#include "Globals.h"
#include "RE_Mesh.h"

ResourceContainer::ResourceContainer(const char* _name, const char * _origin, Resource_Type type, const char* _md5)
{
	origin = (_origin != nullptr) ? _origin : "from Scratch";
	name = (_name != nullptr) ? _name : "unnkown";
	this->type = type;
	if (_md5)
		md5 = _md5;
}

ResourceContainer::~ResourceContainer()
{
}

const char * ResourceContainer::GetName() const
{
	return name;
}

const char * ResourceContainer::GetOrigin() const
{
	return origin;
}

const char* ResourceContainer::GetMD5() const
{
	return md5.c_str();
}

Resource_Type ResourceContainer::GetType() const
{
	return type;
}

void ResourceContainer::SetType(Resource_Type type)
{
	this->type = type;
}

void ResourceContainer::SetMD5(const char * _md5)
{
	md5 = _md5;
}

void ResourceContainer::SetFilePath(const char * path)
{
	from_file = path;
}

const char * ResourceContainer::GetFilePath()
{
	return from_file.c_str();
}
