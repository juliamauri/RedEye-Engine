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
