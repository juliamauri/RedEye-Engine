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
	DEL(name);
	DEL(origin);
	DEL(md5);

	switch (type)
	{
	case R_SHADER:
		DEL(contains.shader);
		break;
	case R_PRIMITIVE:
		DEL(contains.primitive);
		break;
	case R_TEXTURE:
		DEL(contains.texture);
		break;
	case R_MESH:
		DEL(contains.mesh);
		break;
	}
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
	switch (type)
	{
	case R_SHADER:
		break;
	case R_PRIMITIVE:
		break;
	case R_TEXTURE:
		break;
	case R_MESH:
		contains.mesh = (RE_Mesh*)this;
		break;
	}
}

void ResourceContainer::SetMD5(const char * md5)
{
	if (this->md5)
		DEL(this->md5);

	this->md5 = new std::string(md5);
}
