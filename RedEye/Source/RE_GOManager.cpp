#include "RE_GOManager.h"

RE_GOManager::RE_GOManager() {}
RE_GOManager::~RE_GOManager() {}

RE_GameObject* RE_GOManager::AddGO(const char* name, RE_GameObject* parent)
{
	RE_GameObject* goparent = nullptr;
	if (gameObjectsPool.GetCount() > 0)
		goparent = parent ? parent : gameObjectsPool.GetFirstGO();

	RE_GameObject* ret = gameObjectsPool.AtPtr(gameObjectsPool.Push({}));
	ret->SetUp(&componentsPool, name, goparent);
	return ret;
}

RE_GameObject* RE_GOManager::CopyGO(RE_GameObject* copy, RE_GameObject* parent)
{
	RE_GameObject* goparent = nullptr;
	if (gameObjectsPool.GetCount() > 0)
		goparent = parent ? parent : gameObjectsPool.GetFirstGO();

	eastl::list<RE_Component*> copyComponents = copy->GetComponents();

	RE_GameObject* newGO = gameObjectsPool.AtPtr(gameObjectsPool.Push({}));
	newGO->SetUp(&componentsPool, copy->GetName(), goparent);

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

RE_GameObject* RE_GOManager::GetGO(UID id) const
{
	return gameObjectsPool.AtPtr(id);
}

UID RE_GOManager::GetFirstGOUID()
{
	return gameObjectsPool.GetFirstGOUID();
}

RE_GameObject* RE_GOManager::GetFirstGO()
{
	return gameObjectsPool.GetFirstGO();;
}

eastl::vector<UID> RE_GOManager::GetAllGOs()
{
	return gameObjectsPool.GetAllKeys();
}

void RE_GOManager::DestroyGO(UID toDestroy)
{
	RE_GameObject* go = gameObjectsPool.AtPtr(toDestroy);

	eastl::list<RE_GameObject*> childs = go->GetChilds();
	for (auto child : childs) DestroyGO(child->GetPoolID());

	eastl::list<RE_Component*> comps = go->GetComponents();
	for (auto comp : comps) componentsPool.DestroyComponent(comp->GetType(), comp->GetPoolID());

	gameObjectsPool.DeleteGO(toDestroy);
}

RE_GameObject* RE_GOManager::InsertPool(RE_GOManager* pool)
{
	return RepercusiveInsertGO(pool->GetFirstGO(), (TotalGameObjects() > 0) ? GetFirstGO() : nullptr);
}

RE_GOManager* RE_GOManager::GetNewPoolFromID(UID id)
{
	RE_GOManager* ret = new RE_GOManager();

	ret->RepercusiveInsertGO(gameObjectsPool.AtPtr(id), nullptr);

	return ret;
}

eastl::stack<RE_CompLight*> RE_GOManager::GetAllLights(bool check_active)
{
	return componentsPool.GetAllLights(check_active);
}

eastl::vector<const char*> RE_GOManager::GetAllResources()
{
	return componentsPool.GetAllResources();
}

void RE_GOManager::UseResources()
{
	componentsPool.UseResources();
}

void RE_GOManager::UnUseResources()
{
	componentsPool.UnUseResources();
}

void RE_GOManager::ClearPool()
{
	gameObjectsPool.Clear();
	componentsPool.ClearComponents();
}

unsigned int RE_GOManager::GetBinarySize() const
{
	return gameObjectsPool.GetBinarySize() + componentsPool.GetBinarySize();
}

void RE_GOManager::SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources)
{
	gameObjectsPool.SerializeJson(node);
	componentsPool.SerializeJson(node, resources);
}

void RE_GOManager::DeserializeJson(JSONNode* node, eastl::map<int, const char*>* resources)
{
	gameObjectsPool.DeserializeJson(node, &componentsPool);
	componentsPool.DeserializeJson(&gameObjectsPool, node, resources);
}

void RE_GOManager::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources)
{
	gameObjectsPool.SerializeBinary(cursor);
	componentsPool.SerializeBinary(cursor, resources);
}

void RE_GOManager::DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources)
{
	gameObjectsPool.DeserializeBinary(cursor, &componentsPool);
	componentsPool.DeserializeBinary(&gameObjectsPool, cursor, resources);
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

UID GameObjectManager::Push(RE_GameObject val)
{
	UID ret = RE_Math::RandomUID();
	val.SetPoolID(ret);
	PoolMapped::Push(val, ret);
	return ret;
}

UID GameObjectManager::GetFirstGOUID()
{
	return (lastAvaibleIndex > 0) ? pool_[0].GetPoolID() : 0;
}

RE_GameObject* GameObjectManager::GetFirstGO()
{
	return &pool_[0];
}

void GameObjectManager::DeleteGO(UID toDelete)
{
	Pop(toDelete);
}

unsigned int GameObjectManager::GetBinarySize() const
{
	uint size = sizeof(UID);
	size += lastAvaibleIndex * sizeof(UID);
	for (int i = 0; i < lastAvaibleIndex; i++)
		size += pool_[i].GetBinarySize();
	return size;
}

void GameObjectManager::SerializeBinary(char*& cursor)
{
	uint size = sizeof(unsigned int);
	uint goSize = GetCount();
	memcpy(cursor, &goSize, size);
	cursor += size;

	size = sizeof(UID);
	for (uint i = 0; i < goSize; i++) {
		UID goPoolID = pool_[i].GetPoolID();
		memcpy(cursor, &goPoolID, size);
		cursor += size;

		pool_[i].SerializeBinary(cursor);
	}
}

void GameObjectManager::DeserializeBinary(char*& cursor, ComponentsPool* cmpsPool)
{
	eastl::map<UID, RE_GameObject*> idGO;

	uint size = sizeof(unsigned int);
	uint goSize;
	memcpy(&goSize, cursor, size);
	cursor += size;

	size = sizeof(UID);
	for (uint i = 0; i < goSize; i++) {
		RE_GameObject newGO; UID goUID;
		memcpy(&goUID, cursor, size);
		cursor += size;
		newGO.SetPoolID(goUID);
		PoolMapped::Push(newGO, goUID);
		AtPtr(goUID)->DeserializeBinary(cursor, cmpsPool, &idGO);
	}
}

void GameObjectManager::SerializeJson(JSONNode* node)
{
	JSONNode* goPool = node->PushJObject("gameobjects Pool");
	uint goSize = GetCount();
	goPool->PushUInt("gameobjectsSize", goSize);
	for (uint i = 0; i < goSize; i++) {
		JSONNode* goNode = goPool->PushJObject(eastl::to_string(i).c_str());
		goNode->PushUnsignedLongLong("GOUID", pool_[i].GetPoolID());
		pool_[i].SerializeJson(goNode);
		DEL(goNode);
	}
	DEL(goPool);
}

void GameObjectManager::DeserializeJson(JSONNode* node, ComponentsPool* cmpsPool)
{
	eastl::map<UID, RE_GameObject*> idGO;
	JSONNode* goPool = node->PullJObject("gameobjects Pool");
	unsigned int goSize = goPool->PullUInt("gameobjectsSize",  0);

	for (uint i = 0; i < goSize; i++) {
		JSONNode* goNode = goPool->PullJObject(eastl::to_string(i).c_str());
		UID goUID = goNode->PullUnsignedLongLong("GOUID", 0);
		RE_GameObject newGO; newGO.SetPoolID(goUID);
		PoolMapped::Push(newGO, goUID);
		AtPtr(goUID)->DeserializeJSON(goNode, cmpsPool, &idGO);
		DEL(goNode);
	}
	DEL(goPool);
}

void ComponentsPool::ClearComponents()
{
	transPool.Clear();
	camPool.Clear();
	meshPool.Clear();
	lightPool.Clear();
	pCubePool.Clear();
	pDodecahedronPool.Clear();
	pTetrahedronPool.Clear();
	pOctohedronPool.Clear();
	pIcosahedronPool.Clear();
	pSpherePool.Clear();
	pCylinderPool.Clear();
	pHemiSpherePool.Clear();
	pTorusPool.Clear();
	pTrefoiKnotPool.Clear();
	pRockPool.Clear();
	pPlanePool.Clear();
}

RE_Component* ComponentsPool::GetComponent(UID poolid, ComponentType cType)
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
	case C_LIGHT:
		ret = (RE_Component*)lightPool.AtPtr(poolid);
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
	case C_LIGHT:
		ret = (RE_Component*)lightPool.AtPtr(lightPool.GetNewComponent());
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
	case C_LIGHT:
		ret = (RE_Component*)lightPool.AtPtr(lightPool.GetNewComponent());
		((RE_CompLight*)ret)->SetUp(*(RE_CompLight*)cmp, parent);
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

void ComponentsPool::DestroyComponent(ComponentType cType, UID toDelete)
{
	RE_Component* cmp = GetComponent(toDelete, cType);
	cmp->GetGO()->RemoveComponent(cmp);
	switch (cType)
	{
	case C_TRANSFORM: transPool.Pop(toDelete); break;
	case C_CAMERA: camPool.Pop(toDelete); break;
	case C_MESH: meshPool.Pop(toDelete); break;
	case C_LIGHT: lightPool.Pop(toDelete); break;
	case C_CUBE: pCubePool.Pop(toDelete); break;
	case C_DODECAHEDRON: pDodecahedronPool.Pop(toDelete); break;
	case C_TETRAHEDRON: pTetrahedronPool.Pop(toDelete); break;
	case C_OCTOHEDRON: pOctohedronPool.Pop(toDelete); break;
	case C_ICOSAHEDRON: pIcosahedronPool.Pop(toDelete); break;
	case C_SPHERE: pSpherePool.Pop(toDelete); break;
	case C_CYLINDER: pCylinderPool.Pop(toDelete); break;
	case C_HEMISHPERE: pHemiSpherePool.Pop(toDelete); break;
	case C_TORUS: pTorusPool.Pop(toDelete); break;
	case C_TREFOILKNOT: pTrefoiKnotPool.Pop(toDelete); break;
	case C_ROCK: pRockPool.Pop(toDelete); break;
	case C_PLANE: pPlanePool.Pop(toDelete); break;
	}
}

eastl::vector<const char*> ComponentsPool::GetAllResources()
{
	eastl::vector<const char*> allResources;
	eastl::vector<const char*> resources = camPool.GetAllResources();
	if (!resources.empty()) allResources.insert(allResources.end(), resources.begin(), resources.end());
	resources = meshPool.GetAllResources();
	if (!resources.empty()) allResources.insert(allResources.end(), resources.begin(), resources.end());

	eastl::vector<const char*> ret;
	int resSize = 0;
	for (auto res : allResources) {
		bool repeat = false;
		for (auto uniqueRes : ret) {
			resSize = eastl::CharStrlen(res);
			if (resSize > 0 && eastl::Compare(res, uniqueRes, resSize) == 0) {
				repeat = true;
				break;
			}
		}
		if (!repeat) ret.push_back(res);
	}

	return ret;
}

void ComponentsPool::UseResources()
{
	camPool.UseResources();
	meshPool.UseResources();
}

void ComponentsPool::UnUseResources()
{
	camPool.UnUseResources();
	meshPool.UnUseResources();
}

eastl::stack<RE_CompLight*> ComponentsPool::GetAllLights(bool check_active)
{
	eastl::stack<RE_CompLight*> lights;

	eastl::vector<UID> lightsUID = lightPool.GetAllKeys();

	int count = lightsUID.size();
	for (int i = 0; i < count; ++i)
		if (check_active)
		{
			RE_CompLight* light = lightPool.AtPtr(lightsUID[i]);
			if (light->IsActive())
				lights.push(light);
		}
		else
			lights.push(lightPool.AtPtr(lightsUID[i]));

	return lights;
}

unsigned int ComponentsPool::GetBinarySize() const
{
	uint size = 0;
	size += camPool.GetBinarySize();
	size += meshPool.GetBinarySize();
	size += lightPool.GetBinarySize();
	size += pCubePool.GetBinarySize();
	size += pDodecahedronPool.GetBinarySize();
	size += pTetrahedronPool.GetBinarySize();
	size += pOctohedronPool.GetBinarySize();
	size += pIcosahedronPool.GetBinarySize();
	size += pSpherePool.GetBinarySize();
	size += pCylinderPool.GetBinarySize();
	size += pHemiSpherePool.GetBinarySize();
	size += pTorusPool.GetBinarySize();
	size += pTrefoiKnotPool.GetBinarySize();
	size += pRockPool.GetBinarySize();
	size += pPlanePool.GetBinarySize();
	return size;
}

void ComponentsPool::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources)
{
	camPool.SerializeBinary(cursor, resources);
	meshPool.SerializeBinary(cursor, resources);
	lightPool.SerializeBinary(cursor, resources);
	pCubePool.SerializeBinary(cursor, resources);
	pDodecahedronPool.SerializeBinary(cursor, resources);
	pTetrahedronPool.SerializeBinary(cursor, resources);
	pOctohedronPool.SerializeBinary(cursor, resources);
	pIcosahedronPool.SerializeBinary(cursor, resources);
	pSpherePool.SerializeBinary(cursor, resources);
	pCylinderPool.SerializeBinary(cursor, resources);
	pHemiSpherePool.SerializeBinary(cursor, resources);
	pTorusPool.SerializeBinary(cursor, resources);
	pTrefoiKnotPool.SerializeBinary(cursor, resources);
	pRockPool.SerializeBinary(cursor, resources);
	pPlanePool.SerializeBinary(cursor, resources);
}

void ComponentsPool::DeserializeBinary(GameObjectManager* goPool, char*& cursor, eastl::map<int, const char*>* resources)
{
	camPool.DeserializeBinary(goPool, cursor, resources);
	meshPool.DeserializeBinary(goPool, cursor, resources);
	lightPool.DeserializeBinary(goPool, cursor, resources);
	pCubePool.DeserializeBinary(goPool, cursor, resources);
	pDodecahedronPool.DeserializeBinary(goPool, cursor, resources);
	pTetrahedronPool.DeserializeBinary(goPool, cursor, resources);
	pOctohedronPool.DeserializeBinary(goPool, cursor, resources);
	pIcosahedronPool.DeserializeBinary(goPool, cursor, resources);
	pSpherePool.DeserializeBinary(goPool, cursor, resources);
	pCylinderPool.DeserializeBinary(goPool, cursor, resources);
	pHemiSpherePool.DeserializeBinary(goPool, cursor, resources);
	pTorusPool.DeserializeBinary(goPool, cursor, resources);
	pTrefoiKnotPool.DeserializeBinary(goPool, cursor, resources);
	pRockPool.DeserializeBinary(goPool, cursor, resources);
	pPlanePool.DeserializeBinary(goPool, cursor, resources);
}

void ComponentsPool::SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources)
{
	JSONNode* comps = node->PushJObject("Components Pool");
	camPool.SerializeJson(comps, resources);
	meshPool.SerializeJson(comps, resources);
	lightPool.SerializeJson(comps, resources);
	pCubePool.SerializeJson(comps, resources);
	pDodecahedronPool.SerializeJson(comps, resources);
	pTetrahedronPool.SerializeJson(comps, resources);
	pOctohedronPool.SerializeJson(comps, resources);
	pIcosahedronPool.SerializeJson(comps, resources);
	pSpherePool.SerializeJson(comps, resources);
	pCylinderPool.SerializeJson(comps, resources);
	pHemiSpherePool.SerializeJson(comps, resources);
	pTorusPool.SerializeJson(comps, resources);
	pTrefoiKnotPool.SerializeJson(comps, resources);
	pRockPool.SerializeJson(comps, resources);
	pPlanePool.SerializeJson(comps, resources);
	DEL(comps);
}

void ComponentsPool::DeserializeJson(GameObjectManager* goPool, JSONNode* node, eastl::map<int, const char*>* resources)
{
	JSONNode* comps = node->PullJObject("Components Pool");
	camPool.DeserializeJson(goPool, comps, resources);
	meshPool.DeserializeJson(goPool, comps, resources);
	lightPool.DeserializeJson(goPool, comps, resources);
	pCubePool.DeserializeJson(goPool, comps, resources);
	pDodecahedronPool.DeserializeJson(goPool, comps, resources);
	pTetrahedronPool.DeserializeJson(goPool, comps, resources);
	pOctohedronPool.DeserializeJson(goPool, comps, resources);
	pIcosahedronPool.DeserializeJson(goPool, comps, resources);
	pSpherePool.DeserializeJson(goPool, comps, resources);
	pCylinderPool.DeserializeJson(goPool, comps, resources);
	pHemiSpherePool.DeserializeJson(goPool, comps, resources);
	pTorusPool.DeserializeJson(goPool, comps, resources);
	pTrefoiKnotPool.DeserializeJson(goPool, comps, resources);
	pRockPool.DeserializeJson(goPool, comps, resources);
	pPlanePool.DeserializeJson(goPool, comps, resources);
	DEL(comps);
}
