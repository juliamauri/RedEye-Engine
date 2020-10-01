#pragma once
#include "Module.h"

#include "PoolMapped.h"

class GameObjectManager : public PoolMapped<RE_GameObject, int> {
public:
	GameObjectManager() { }
	~GameObjectManager() { }

	void Clear();

	int Push(RE_GameObject val)override;
};

class RE_GOManager :
    public Module
{
public:
	RE_GOManager(const char* name, bool start_enabled = true);
	~RE_GOManager();

	RE_GameObject* AddGOToScene(const char* name, RE_GameObject* parent);
	GameObjectManager sceneGOs;
};