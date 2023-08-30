#include "RE_GameObjectPool.h"

#include "RE_Memory.h"
#include "Application.h"
#include "RE_Random.h"
#include "RE_Json.h"
#include "RE_ComponentsPool.h"

void GameObjectsPool::Clear()
{
	key_map.clear();
	lastAvaibleIndex = 0;
}

GO_UID GameObjectsPool::Push(RE_GameObject val)
{
	return RE_HashMap::Push(val, val.go_uid = RANDOM_UID) ? val.go_uid : 0;
}

GO_UID GameObjectsPool::GetNewGOUID()
{
	return Push({});
}

RE_GameObject* GameObjectsPool::GetNewGOPtr()
{
	return AtPtr(Push({}));
}

GO_UID GameObjectsPool::GetRootUID() const
{
	return (lastAvaibleIndex > 0) ? pool_[0].go_uid : 0;
}

RE_GameObject* GameObjectsPool::GetRootPtr() const
{
	return (lastAvaibleIndex > 0) ? &pool_[0] : nullptr;
}

const RE_GameObject* GameObjectsPool::GetRootCPtr() const
{
	return (lastAvaibleIndex > 0) ? &pool_[0] : nullptr;
}

void GameObjectsPool::DeleteGO(GO_UID toDelete)
{
	Pop(toDelete);
}

eastl::vector<RE_GameObject*> GameObjectsPool::GetAllPtrs() const
{
	eastl::vector<RE_GameObject*> ret;
	eastl::vector<GO_UID> uids = GetAllKeys();
	for (auto uid : uids) ret.push_back(AtPtr(uid));
	return ret;
}

eastl::vector<eastl::pair<const GO_UID, RE_GameObject*>> GameObjectsPool::GetAllData() const
{
	eastl::vector<eastl::pair<const GO_UID, RE_GameObject*>> ret;
	eastl::vector<GO_UID> uids = GetAllKeys();
	for (auto uid : uids) ret.push_back({ uid, AtPtr(uid) });
	return ret;
}

size_t GameObjectsPool::GetBinarySize() const
{
	size_t size = sizeof(unsigned int);
	size += lastAvaibleIndex * sizeof(GO_UID);
	for (int i = 0; i < lastAvaibleIndex; i++)
		size += pool_[i].GetBinarySize();
	return size;
}

void GameObjectsPool::SerializeBinary(char*& cursor)
{
	size_t size = sizeof(unsigned int);
	size_t goSize = GetCount();
	memcpy(cursor, &goSize, size);
	cursor += size;

	size = sizeof(GO_UID);
	for (size_t i = 0; i < goSize; i++)
	{
		memcpy(cursor, &pool_[i].go_uid, size);
		cursor += size;
		pool_[i].SerializeBinary(cursor);
	}
}

void GameObjectsPool::DeserializeBinary(char*& cursor, ComponentsPool* cmpsPool)
{
	eastl::map<GO_UID, RE_GameObject*> idGO;

	uint size = sizeof(unsigned int);
	uint goSize;
	memcpy(&goSize, cursor, size);
	cursor += size;

	size = sizeof(GO_UID);
	for (uint i = 0; i < goSize; i++)
	{
		RE_GameObject newGO; GO_UID goUID;
		memcpy(&goUID, cursor, size);
		cursor += size;
		newGO.go_uid = goUID;
		RE_HashMap::Push(newGO, goUID);
		AtPtr(goUID)->DeserializeBinary(cursor, this, cmpsPool);
	}
}

void GameObjectsPool::SerializeJson(RE_Json* node)
{
	RE_Json* goPool = node->PushJObject("gameobjects Pool");
	size_t goSize = GetCount();
	goPool->PushSizeT("gameobjectsSize", goSize);
	for (size_t i = 0; i < goSize; i++)
	{
		RE_Json* goNode = goPool->PushJObject(eastl::to_string(i).c_str());
		goNode->Push("GOUID", pool_[i].go_uid);
		pool_[i].SerializeJson(goNode);
		DEL(goNode)
	}
	DEL(goPool)
}

void GameObjectsPool::DeserializeJson(RE_Json* node, ComponentsPool* cmpsPool)
{
	eastl::map<GO_UID, RE_GameObject*> idGO;
	RE_Json* goPool = node->PullJObject("gameobjects Pool");
	auto goSize = goPool->PullSizeT("gameobjectsSize", 0);

	for (size_t i = 0; i < goSize; i++)
	{
		RE_Json* goNode = goPool->PullJObject(eastl::to_string(i).c_str());
		GO_UID goUID = goNode->PullUnsignedLongLong("GOUID", 0);
		RE_GameObject newGO;
		newGO.go_uid = goUID;
		RE_HashMap::Push(newGO, goUID);
		AtPtr(goUID)->DeserializeJSON(goNode, this, cmpsPool);
		DEL(goNode)
	}
	DEL(goPool)
}

eastl::vector<GO_UID> GameObjectsPool::GetAllKeys() const
{
	eastl::vector<GO_UID> ret;
	for (auto &go : key_map) ret.push_back(go.first);
	return ret;
}