#include "RE_Component.h"

#include "RE_ECS_Pool.h"

UID RE_Component::PoolSetUp(GameObjectsPool* pool, const UID parent, bool report_parent)
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
