#include "RE_Component.h"

RE_Component::RE_Component(ComponentType type, RE_GameObject * go, bool start_active)
	: type(type), go(go), active(start_active)
{}

bool RE_Component::IsActive() const
{
	return active;
}

void RE_Component::SetActive(bool value)
{
	active = value;
}

ComponentType RE_Component::GetType() const
{
	return type;
}

RE_GameObject * RE_Component::GetGO() const
{
	return go;
}
