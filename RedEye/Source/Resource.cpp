#include "Resource.h"

#include "Globals.h"
#include "RE_Mesh.h"

ResourceContainer::ResourceContainer(const char* _name, const char * _origin, Resource_Type type, const char* _md5)
{
	origin = (_origin != nullptr) ? _origin : "from Scratch";
	name = (_name != nullptr) ? _name : "unnkown";
	this->type = type;
	if (_md5)
	{
		std::string str(_md5);
		char* writtable = new char[str.size() + 1];
		std::copy(str.begin(), str.end(), writtable);
		writtable[str.size()] = '\0';
		md5 = writtable;
	}
}

ResourceContainer::~ResourceContainer()
{
	if (md5)
		DEL_A(md5);
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

void ResourceContainer::SetMD5(const char * _md5)
{
	if (md5)
	{
		DEL_A(md5);
		md5 = nullptr;
	}
	if (_md5)
	{
		std::string str(_md5);
		char* writtable = new char[str.size() + 1];
		std::copy(str.begin(), str.end(), writtable);
		writtable[str.size()] = '\0';
		md5 = writtable;
	}
}

void ResourceContainer::SetFilePath(const char * path)
{
	from_file = path;
}

const char * ResourceContainer::GetFilePath()
{
	return from_file.c_str();
}
