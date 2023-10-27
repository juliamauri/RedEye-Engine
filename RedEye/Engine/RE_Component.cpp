#include "RE_Component.h"

#include "RE_ECS_Pool.h"

COMP_UID RE_Component::PoolSetUp(GameObjectsPool* pool, const GO_UID parent, bool report_parent)
{
	pool_gos = pool;
	useParent = (go = parent);
	if (report_parent && useParent) pool_gos->AtPtr(go)->ReportComponent(id, type);
	return id;
}

RE_GameObject* RE_Component::GetGOPtr() const
{
	return pool_gos->AtPtr(go);
}

const RE_GameObject* RE_Component::GetGOCPtr() const
{
	return pool_gos->AtCPtr(go);
}

math::vec RE_Component::GetGlobalPosition() const
{
	return dynamic_cast<RE_CompTransform*>(pool_gos->AtCPtr(go)->GetCompPtr(RE_Component::Type::TRANSFORM))->GetGlobalPosition();
}

void RE_Component::JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources) const
{
	DEL(node)
}

void RE_Component::JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources)
{
	DEL(node)
}
