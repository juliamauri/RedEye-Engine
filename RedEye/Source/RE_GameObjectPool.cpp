#include "RE_GameObjectPool.h"

#include "Application.h"
#include "RE_Math.h"
#include "RE_Json.h"

void GameObjectsPool::Clear()
{
	poolmapped_.clear();
	lastAvaibleIndex = 0;
}

UID GameObjectsPool::Push(RE_GameObject val)
{
	UID ret = 0;
	if (RE_HashMap::Push(val, val.go_uid = RE_MATH->RandomUID())) ret = val.go_uid;
	return ret;
}

UID GameObjectsPool::GetNewGOUID()
{
	return Push({});
}

RE_GameObject* GameObjectsPool::GetNewGOPtr()
{
	return AtPtr(Push({}));
}

UID GameObjectsPool::GetRootUID() const
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

void GameObjectsPool::DeleteGO(UID toDelete)
{
	Pop(toDelete);
}

eastl::vector<RE_GameObject*> GameObjectsPool::GetAllPtrs() const
{
	eastl::vector<RE_GameObject*> ret;
	eastl::vector<UID> uids = GetAllKeys();
	for (auto uid : uids) ret.push_back(AtPtr(uid));
	return ret;
}

eastl::vector<eastl::pair<const UID, RE_GameObject*>> GameObjectsPool::GetAllData() const
{
	eastl::vector<eastl::pair<const UID, RE_GameObject*>> ret;
	eastl::vector<UID> uids = GetAllKeys();
	for (auto uid : uids) ret.push_back({ uid, AtPtr(uid) });
	return ret;
}

unsigned int GameObjectsPool::GetBinarySize() const
{
	uint size = sizeof(unsigned int);
	size += lastAvaibleIndex * sizeof(UID);
	for (int i = 0; i < lastAvaibleIndex; i++)
		size += pool_[i].GetBinarySize();
	return size;
}

void GameObjectsPool::SerializeBinary(char*& cursor)
{
	uint size = sizeof(unsigned int);
	uint goSize = GetCount();
	memcpy(cursor, &goSize, size);
	cursor += size;

	size = sizeof(UID);
	for (uint i = 0; i < goSize; i++)
	{
		memcpy(cursor, &pool_[i].go_uid, size);
		cursor += size;
		pool_[i].SerializeBinary(cursor);
	}
}

void GameObjectsPool::DeserializeBinary(char*& cursor, ComponentsPool* cmpsPool)
{
	eastl::map<UID, RE_GameObject*> idGO;

	uint size = sizeof(unsigned int);
	uint goSize;
	memcpy(&goSize, cursor, size);
	cursor += size;

	size = sizeof(UID);
	for (uint i = 0; i < goSize; i++)
	{
		RE_GameObject newGO; UID goUID;
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
	uint goSize = GetCount();
	goPool->PushUInt("gameobjectsSize", goSize);
	for (uint i = 0; i < goSize; i++)
	{
		RE_Json* goNode = goPool->PushJObject(eastl::to_string(i).c_str());
		goNode->PushUnsignedLongLong("GOUID", pool_[i].go_uid);
		pool_[i].SerializeJson(goNode);
		DEL(goNode);
	}
	DEL(goPool);
}

void GameObjectsPool::DeserializeJson(RE_Json* node, ComponentsPool* cmpsPool)
{
	eastl::map<UID, RE_GameObject*> idGO;
	RE_Json* goPool = node->PullJObject("gameobjects Pool");
	unsigned int goSize = goPool->PullUInt("gameobjectsSize", 0);

	for (uint i = 0; i < goSize; i++) {
		RE_Json* goNode = goPool->PullJObject(eastl::to_string(i).c_str());
		UID goUID = goNode->PullUnsignedLongLong("GOUID", 0);
		RE_GameObject newGO;
		newGO.go_uid = goUID;
		RE_HashMap::Push(newGO, goUID);
		AtPtr(goUID)->DeserializeJSON(goNode, this, cmpsPool);
		DEL(goNode);
	}
	DEL(goPool);
}

eastl::vector<UID> GameObjectsPool::GetAllKeys() const
{
	eastl::vector<UID> ret;
	for (auto go : poolmapped_) ret.push_back(go.first);
	return ret;
}