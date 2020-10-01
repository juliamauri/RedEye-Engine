#include "RE_GOManager.h"

#include "RE_GameObject.h"

RE_GOManager::RE_GOManager(const char* name, bool start_enabled) : Module(name, start_enabled)
{
}

RE_GOManager::~RE_GOManager()
{
}

RE_GameObject* RE_GOManager::AddGOToScene(const char* name, RE_GameObject* parent)
{
	RE_GameObject* goparent = nullptr;
	if (sceneGOs.GetCount() > 0)
		goparent = parent ? parent : sceneGOs.AtPtr(0);

	return sceneGOs.AtPtr(sceneGOs.Push({ name, GUID_NULL, goparent }));
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