#include "RE_Component.h"

RE_Component::RE_Component(const ComponentType type, RE_GameObject * go, const bool start_active)
	: type(type), go(go), active(start_active)
{}

bool RE_Component::IsActive() const { return active; }

void RE_Component::SetActive(const bool value) { active = value; }

ComponentType RE_Component::GetType() const { return type; }

RE_GameObject * RE_Component::GetGO() const { return go; }

RE_Component * RE_Component::AsComponent() const { return (RE_Component*)this; }

void RE_Component::Serialize(JSONNode * node, rapidjson::Value * val)
{}
