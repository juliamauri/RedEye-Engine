#include "Resource.h"

#include "Globals.h"
#include "RE_Mesh.h"

ResourceContainer::ResourceContainer(const char* _name, const char * _origin, Resource_Type type, unsigned int id)
{
	origin = (_origin != nullptr) ? _origin : "from Scratch";
	name = (_name != nullptr) ? _name : "unnkown";
	this->type = type;
	this->id = id;
}

ResourceContainer::~ResourceContainer()
{
	DEL(name);
	DEL(origin);

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

unsigned int ResourceContainer::GetID() const
{
	return id;
}

Resource_Type ResourceContainer::GetType() const
{
	return type;
}

void ResourceContainer::SetID(unsigned int id)
{
	this->id = id;
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
