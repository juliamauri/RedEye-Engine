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
		default:
			componentsPool.CopyComponent(copyC, newGO);
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
	case C_CUBE:
		ret = (RE_Component*)pCubePool.AtPtr(poolid);
		break;
	case C_DODECAHEDRON:
		ret = (RE_Component*)pDodecahedronPool.AtPtr(poolid);
		break;
	case C_TETRAHEDRON:
		ret = (RE_Component*)pTetrahedronPool.AtPtr(poolid);
		break;
	case C_OCTOHEDRON:
		ret = (RE_Component*)pOctohedronPool.AtPtr(poolid);
		break;
	case C_ICOSAHEDRON:
		ret = (RE_Component*)pIcosahedronPool.AtPtr(poolid);
		break;
	case C_SPHERE:
		ret = (RE_Component*)pSpherePool.AtPtr(poolid);
		break;
	case C_CYLINDER:
		ret = (RE_Component*)pCylinderPool.AtPtr(poolid);
		break;
	case C_HEMISHPERE:
		ret = (RE_Component*)pHemiSpherePool.AtPtr(poolid);
		break;
	case C_TORUS:
		ret = (RE_Component*)pTorusPool.AtPtr(poolid);
		break;
	case C_TREFOILKNOT:
		ret = (RE_Component*)pTrefoiKnotPool.AtPtr(poolid);
		break;
	case C_ROCK:
		ret = (RE_Component*)pRockPool.AtPtr(poolid);
		break;
	case C_PLANE:
		ret = (RE_Component*)pPlanePool.AtPtr(poolid);
		break;
	}
	return ret;
}

RE_Component* ComponentsPool::GetNewComponent(ComponentType cType)
{
	RE_Component* ret = nullptr;
	switch (cType)
	{
	case C_TRANSFORM:
		ret = (RE_Component*)transPool.AtPtr(transPool.GetNewComponent());
		break;
	case C_CAMERA:
		ret = (RE_Component*)camPool.AtPtr(camPool.GetNewComponent());
		break;
	case C_MESH:
		ret = (RE_Component*)meshPool.AtPtr(meshPool.GetNewComponent());
		break;
	case C_CUBE:
		ret = (RE_Component*)pCubePool.AtPtr(pCubePool.GetNewComponent());
		break;
	case C_DODECAHEDRON:
		ret = (RE_Component*)pDodecahedronPool.AtPtr(pDodecahedronPool.GetNewComponent());
		break;
	case C_TETRAHEDRON:
		ret = (RE_Component*)pTetrahedronPool.AtPtr(pTetrahedronPool.GetNewComponent());
		break;
	case C_OCTOHEDRON:
		ret = (RE_Component*)pOctohedronPool.AtPtr(pOctohedronPool.GetNewComponent());
		break;
	case C_ICOSAHEDRON:
		ret = (RE_Component*)pIcosahedronPool.AtPtr(pIcosahedronPool.GetNewComponent());
		break;
	case C_SPHERE:
		ret = (RE_Component*)pSpherePool.AtPtr(pSpherePool.GetNewComponent());
		break;
	case C_CYLINDER:
		ret = (RE_Component*)pCylinderPool.AtPtr(pCylinderPool.GetNewComponent());
		break;
	case C_HEMISHPERE:
		ret = (RE_Component*)pHemiSpherePool.AtPtr(pHemiSpherePool.GetNewComponent());
		break;
	case C_TORUS:
		ret = (RE_Component*)pTorusPool.AtPtr(pTorusPool.GetNewComponent());
		break;
	case C_TREFOILKNOT:
		ret = (RE_Component*)pTrefoiKnotPool.AtPtr(pTrefoiKnotPool.GetNewComponent());
		break;
	case C_ROCK:
		ret = (RE_Component*)pRockPool.AtPtr(pRockPool.GetNewComponent());
		break;
	case C_PLANE:
		ret = (RE_Component*)pPlanePool.AtPtr(pPlanePool.GetNewComponent());
		break;
	}
	return ret;
}

RE_Component* ComponentsPool::CopyComponent(RE_Component* cmp, RE_GameObject* parent)
{
	RE_Component* ret = nullptr;
	switch (cmp->GetType())
	{
	case C_TRANSFORM:
		ret = (RE_Component*)transPool.AtPtr(transPool.GetNewComponent());
		((RE_CompTransform*)ret)->SetUp(*(RE_CompTransform*)cmp, parent);
		break;
	case C_CAMERA:
		ret = (RE_Component*)camPool.AtPtr(camPool.GetNewComponent());
		((RE_CompCamera*)ret)->SetUp(*(RE_CompCamera*)cmp, parent);
		break;
	case C_MESH:
		ret = (RE_Component*)meshPool.AtPtr(meshPool.GetNewComponent());
		((RE_CompMesh*)ret)->SetUp(*(RE_CompMesh*)cmp, parent);
		break;
	case C_CUBE:
		ret = (RE_Component*)pCubePool.AtPtr(pCubePool.GetNewComponent());
		((RE_CompCube*)ret)->SetUp(*(RE_CompCube*)cmp, parent);
		break;
	case C_DODECAHEDRON:
		ret = (RE_Component*)pDodecahedronPool.AtPtr(pDodecahedronPool.GetNewComponent());
		((RE_CompDodecahedron*)ret)->SetUp(*(RE_CompDodecahedron*)cmp, parent);
		break;
	case C_TETRAHEDRON:
		ret = (RE_Component*)pTetrahedronPool.AtPtr(pTetrahedronPool.GetNewComponent());
		((RE_CompTetrahedron*)ret)->SetUp(*(RE_CompTetrahedron*)cmp, parent);
		break;
	case C_OCTOHEDRON:
		ret = (RE_Component*)pOctohedronPool.AtPtr(pOctohedronPool.GetNewComponent());
		((RE_CompOctohedron*)ret)->SetUp(*(RE_CompOctohedron*)cmp, parent);
		break;
	case C_ICOSAHEDRON:
		ret = (RE_Component*)pIcosahedronPool.AtPtr(pIcosahedronPool.GetNewComponent());
		((RE_CompIcosahedron*)ret)->SetUp(*(RE_CompIcosahedron*)cmp, parent);
		break;
	case C_SPHERE:
		ret = (RE_Component*)pSpherePool.AtPtr(pSpherePool.GetNewComponent());
		((RE_CompSphere*)ret)->SetUp(*(RE_CompSphere*)cmp, parent);
		break;
	case C_CYLINDER:
		ret = (RE_Component*)pCylinderPool.AtPtr(pCylinderPool.GetNewComponent());
		((RE_CompCylinder*)ret)->SetUp(*(RE_CompCylinder*)cmp, parent);
		break;
	case C_HEMISHPERE:
		ret = (RE_Component*)pHemiSpherePool.AtPtr(pHemiSpherePool.GetNewComponent());
		((RE_CompHemiSphere*)ret)->SetUp(*(RE_CompHemiSphere*)cmp, parent);
		break;
	case C_TORUS:
		ret = (RE_Component*)pTorusPool.AtPtr(pTorusPool.GetNewComponent());
		((RE_CompTorus*)ret)->SetUp(*(RE_CompTorus*)cmp, parent);
		break;
	case C_TREFOILKNOT:
		ret = (RE_Component*)pTrefoiKnotPool.AtPtr(pTrefoiKnotPool.GetNewComponent());
		((RE_CompTrefoiKnot*)ret)->SetUp(*(RE_CompTrefoiKnot*)cmp, parent);
		break;
	case C_ROCK:
		ret = (RE_Component*)pRockPool.AtPtr(pRockPool.GetNewComponent());
		((RE_CompRock*)ret)->SetUp(*(RE_CompRock*)cmp, parent);
		break;
	case C_PLANE:
		ret = (RE_Component*)pPlanePool.AtPtr(pPlanePool.GetNewComponent());
		((RE_CompPlane*)ret)->SetUp(*(RE_CompPlane*)cmp, parent);
		break;
	}
	return ret;
}

void ComponentsPool::DeleteTransform(int id)
{
	transPool.Pop(id);
}