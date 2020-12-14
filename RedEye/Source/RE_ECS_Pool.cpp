#include "RE_ECS_Pool.h"

#include "Application.h"
#include "RE_Json.h"
#include "ModuleInput.h"
#include "ModuleScene.h"

void RE_ECS_Pool::Update()
{
	componentsPool.Update();
}

RE_GameObject* RE_ECS_Pool::AddGO(const char* name, UID parent, bool broadcast)
{
	if (gameObjectsPool.GetCount() > 0 && !parent)
		parent = gameObjectsPool.GetRootUID();

	UID new_go_uid = gameObjectsPool.GetNewGOUID();
	RE_GameObject* ret = gameObjectsPool.AtPtr(new_go_uid);
	ret->SetUp(&gameObjectsPool, &componentsPool, name, parent);

	if (broadcast) RE_INPUT->Push(GO_HAS_NEW_CHILD, RE_SCENE, parent, new_go_uid);

	return ret;
}

UID RE_ECS_Pool::CopyGO(const RE_GameObject* copy, UID parent, bool broadcast)
{
	UID new_go = AddGO(copy->name.c_str(), parent, broadcast)->GetUID();

	for (auto copy_comp : copy->AllCompData())
		componentsPool.CopyComponent(&gameObjectsPool, copy->pool_comps->GetComponentPtr(copy_comp.uid, static_cast<ComponentType>(copy_comp.type)), new_go);
	
	return new_go;
}

UID RE_ECS_Pool::CopyGOandChilds(const RE_GameObject* copy, UID parent, bool broadcast)
{
	UID ret_uid = CopyGO(copy, parent, broadcast);

	eastl::stack<eastl::pair<const RE_GameObject*, UID>> copy_gos;
	for (auto copy_child : copy->GetChildsCPtr())
		copy_gos.push({ copy_child, ret_uid });

	while (!copy_gos.empty())
	{
		eastl::pair<const RE_GameObject*, UID> copy_go = copy_gos.top();
		copy_gos.pop();

		UID new_go_uid = CopyGO(copy_go.first, copy_go.second);

		for (auto copy_child : copy_go.first->GetChildsCPtr())
			copy_gos.push({ copy_child, new_go_uid });
	}

	return ret_uid;
}

RE_GameObject* RE_ECS_Pool::GetGOPtr(UID id) const
{
	return gameObjectsPool.AtPtr(id);
}

const RE_GameObject* RE_ECS_Pool::GetGOCPtr(UID id) const
{
	return gameObjectsPool.AtCPtr(id);
}

UID RE_ECS_Pool::GetRootUID() const
{
	return gameObjectsPool.GetRootUID();
}

RE_GameObject* RE_ECS_Pool::GetRootPtr() const
{
	return gameObjectsPool.GetRootPtr();
}

const RE_GameObject* RE_ECS_Pool::GetRootCPtr() const
{
	return gameObjectsPool.GetRootCPtr();
}

eastl::vector<UID> RE_ECS_Pool::GetAllGOUIDs() const
{
	return gameObjectsPool.GetAllKeys();
}

eastl::vector<RE_GameObject*> RE_ECS_Pool::GetAllGOPtrs() const
{
	return gameObjectsPool.GetAllPtrs();
}

eastl::vector<eastl::pair<const UID, RE_GameObject*>> RE_ECS_Pool::GetAllGOData() const
{
	return gameObjectsPool.GetAllData();
}

void RE_ECS_Pool::DestroyGO(UID toDestroy)
{
	RE_GameObject* go = gameObjectsPool.AtPtr(toDestroy);
	go->UnlinkParent();

	eastl::stack<UID> gos;
	for (auto child : go->childs) gos.push(child);
	for (auto comp : go->AllCompData()) componentsPool.DestroyComponent(static_cast<ComponentType>(comp.type), comp.uid);

	while (!gos.empty()) {

		UID toD = gos.top();
		gos.pop();

		RE_GameObject* cGO = gameObjectsPool.AtPtr(toD);

		for (auto child : cGO->childs) gos.push(child);
		for (auto comp : cGO->AllCompData()) componentsPool.DestroyComponent(static_cast<ComponentType>(comp.type), comp.uid);

		gameObjectsPool.DeleteGO(toD);
	}

	gameObjectsPool.DeleteGO(toDestroy);
}

UID RE_ECS_Pool::InsertPool(RE_ECS_Pool* pool, bool broadcast)
{
	return CopyGOandChilds(pool->GetRootPtr(), (TotalGameObjects() > 0) ? GetRootUID() : 0, broadcast);
}

eastl::vector<UID> RE_ECS_Pool::GetAllCompUID(ushortint type) const
{
	return componentsPool.GetAllCompUID(type);
}

eastl::vector<RE_Component*> RE_ECS_Pool::GetAllCompPtr(ushortint type) const
{
	return componentsPool.GetAllCompPtr(type);
}

eastl::vector<const RE_Component*> RE_ECS_Pool::GetAllCompCPtr(ushortint type) const
{
	return componentsPool.GetAllCompCPtr(type);
}

eastl::vector<eastl::pair<const UID, RE_Component*>> RE_ECS_Pool::GetAllCompData(ushortint type) const
{
	return componentsPool.GetAllCompData(type);
}

RE_Component* RE_ECS_Pool::GetComponentPtr(UID poolid, ComponentType cType)
{
	return componentsPool.GetComponentPtr(poolid, cType);
}

const RE_Component* RE_ECS_Pool::GetComponentCPtr(UID poolid, ComponentType cType) const
{
	return componentsPool.GetComponentCPtr(poolid, cType);
}

RE_ECS_Pool* RE_ECS_Pool::GetNewPoolFromID(UID id)
{
	RE_ECS_Pool* ret = new RE_ECS_Pool();

	ret->CopyGOandChilds(gameObjectsPool.AtPtr(id), 0);

	return ret;
}

eastl::vector<const char*> RE_ECS_Pool::GetAllResources()
{
	return componentsPool.GetAllResources();
}

void RE_ECS_Pool::UseResources()
{
	componentsPool.UseResources();
}

void RE_ECS_Pool::UnUseResources()
{
	componentsPool.UnUseResources();
}

void RE_ECS_Pool::ClearPool()
{
	gameObjectsPool.Clear();
	componentsPool.ClearComponents();
}

unsigned int RE_ECS_Pool::GetBinarySize() const
{
	return gameObjectsPool.GetBinarySize() + componentsPool.GetBinarySize();
}

void RE_ECS_Pool::SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources)
{
	gameObjectsPool.SerializeJson(node);
	componentsPool.SerializeJson(node, resources);
}

void RE_ECS_Pool::DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources)
{
	gameObjectsPool.DeserializeJson(node, &componentsPool);
	componentsPool.DeserializeJson(&gameObjectsPool, node, resources);
}

void RE_ECS_Pool::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources)
{
	gameObjectsPool.SerializeBinary(cursor);
	componentsPool.SerializeBinary(cursor, resources);
}

void RE_ECS_Pool::DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources)
{
	gameObjectsPool.DeserializeBinary(cursor, &componentsPool);
	componentsPool.DeserializeBinary(&gameObjectsPool, cursor, resources);
}
