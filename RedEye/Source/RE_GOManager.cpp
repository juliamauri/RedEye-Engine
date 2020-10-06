#include "RE_GOManager.h"

#include "RE_GameObject.h"

RE_GOManager::RE_GOManager()
{
}

RE_GOManager::~RE_GOManager()
{
}

RE_GameObject* RE_GOManager::AddGO(const char* name, RE_GameObject* parent)
{
	RE_GameObject* goparent = nullptr;
	if (gameObjectsPool.GetCount() > 0)
		goparent = parent ? parent : gameObjectsPool.AtPtr(0);

	RE_GameObject* ret = gameObjectsPool.AtPtr(gameObjectsPool.Push({}));
	ret->SetUp(&componentsPool, name, GUID_NULL, goparent);
	return ret;
}

RE_GameObject* RE_GOManager::AddGO(const char* name, UUID uuid, RE_GameObject* parent)
{
	RE_GameObject* goparent = nullptr;
	if (gameObjectsPool.GetCount() > 0)
		goparent = parent ? parent : gameObjectsPool.AtPtr(0);

	RE_GameObject* ret = gameObjectsPool.AtPtr(gameObjectsPool.Push({}));
	ret->SetUp(&componentsPool, name, uuid, goparent);
	return ret;
}

RE_GameObject* RE_GOManager::CopyGO(RE_GameObject* copy, RE_GameObject* parent)
{
	RE_GameObject* goparent = nullptr;
	if (gameObjectsPool.GetCount() > 0)
		goparent = parent ? parent : gameObjectsPool.AtPtr(0);

	eastl::list<RE_Component*> copyComponents = copy->GetComponents();

	RE_GameObject* newGO = gameObjectsPool.AtPtr(gameObjectsPool.Push({}));
	newGO->SetUp(&componentsPool, copy->GetName(), GUID_NULL, goparent);

	for (RE_Component* copyC : copyComponents) {
		switch (copyC->GetType())
		{
		case ComponentType::C_TRANSFORM:
			newGO->GetTransform()->SetPosition(copy->GetTransform()->GetLocalPosition());
			newGO->GetTransform()->SetScale(copy->GetTransform()->GetLocalScale());
			newGO->GetTransform()->SetRotation(copy->GetTransform()->GetLocalQuaternionRotation());
			break;
		case ComponentType::C_CAMERA:
			componentsPool.CopyCamera((RE_CompCamera*)copyC, newGO);
			break;
		case ComponentType::C_MESH:
			componentsPool.CopyMesh((RE_CompMesh*)copyC, newGO);
			break;
		}
	}
	return newGO;
}

RE_GameObject* RE_GOManager::GetGO(int id) const
{
	return gameObjectsPool.AtPtr(id);
}

eastl::vector<int> RE_GOManager::GetAllGOs()
{
	return gameObjectsPool.GetAllKeys();
}

RE_GameObject* RE_GOManager::InsertPool(RE_GOManager* pool)
{
	return RepercusiveInsertGO(pool->GetGO(0), (TotalGameObjects() > 0) ? GetGO(0) : nullptr);
}

RE_GOManager* RE_GOManager::GetNewPoolFromID(int id)
{
	RE_GOManager* ret = new RE_GOManager();

	ret->RepercusiveInsertGO(gameObjectsPool.AtPtr(id), nullptr);

	return ret;
}

void RE_GOManager::ClearPool()
{
	gameObjectsPool.Clear();
	componentsPool.ClearComponents();
}

RE_GameObject* RE_GOManager::RepercusiveInsertGO(RE_GameObject* go, RE_GameObject* parent)
{
	RE_GameObject* ret = CopyGO(go, parent);

	eastl::list<RE_GameObject*> childs = go->GetChilds();
	if (!childs.empty())
		for (RE_GameObject* c : childs)
			RepercusiveInsertGO(c, ret);

	return ret;
}

void GameObjectManager::Clear()
{
	poolmapped_.clear();
	lastAvaibleIndex = 0;
}

int GameObjectManager::Push(RE_GameObject val)
{
	int ret = lastAvaibleIndex;
	val.SetPoolID(ret);
	PoolMapped::Push(val, ret);
	return ret;
}

void ComponentsPool::ClearComponents()
{
	transPool.Clear();
	camPool.Clear();
	meshPool.Clear();
}

RE_Component* ComponentsPool::GetComponent(int poolid, ComponentType cType)
{
	RE_Component* ret = nullptr;
	switch (cType)
	{
	case C_TRANSFORM:
		ret = (RE_Component*)transPool.AtPtr(poolid);
		break;
	case C_CAMERA:
		ret = (RE_Component*)camPool.AtPtr(poolid);
	break;
	case C_MESH:
		ret = (RE_Component*)meshPool.AtPtr(poolid);
		break;
	}
	return ret;
}

RE_CompTransform* ComponentsPool::GetNewTransform()
{
	return transPool.AtPtr(transPool.GetNewComponent());
}

RE_CompTransform* ComponentsPool::CopyTransform(RE_CompTransform* transComp, RE_GameObject* parent)
{
	RE_CompTransform* newTransform = transPool.AtPtr(transPool.GetNewComponent());
	newTransform->SetUp(*transComp, parent);
	return newTransform;
}

void ComponentsPool::DeleteTransform(int id)
{
	transPool.Pop(id);
}

RE_CompCamera* ComponentsPool::GetNewCamera()
{
	return camPool.AtPtr(camPool.GetNewComponent());
}

RE_CompCamera* ComponentsPool::CopyCamera(RE_CompCamera* camComp, RE_GameObject* parent)
{
	RE_CompCamera* newCamera = camPool.AtPtr(camPool.GetNewComponent());
	newCamera->SetUp(*camComp, parent);
	return newCamera;
}

RE_CompMesh* ComponentsPool::GetNewMesh()
{
	return meshPool.AtPtr(meshPool.GetNewComponent());;
}

RE_CompMesh* ComponentsPool::CopyMesh(RE_CompMesh* meshComp, RE_GameObject* parent)
{
	RE_CompMesh* newMesh = meshPool.AtPtr(meshPool.GetNewComponent());
	newMesh->SetUp(*meshComp, parent);
	return newMesh;
}