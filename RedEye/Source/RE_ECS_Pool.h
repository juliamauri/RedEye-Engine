#pragma once

#include "RE_ComponentsPool.h"
#include "RE_GameObjectPool.h"

class RE_Json;

class RE_ECS_Pool
{
public:
	RE_ECS_Pool() {}
	~RE_ECS_Pool() {}

	// Content iteration
	void Update();
	void ClearPool();

	// Pool handling
	RE_GameObject* AddGO(const char* name, UID parent, bool broadcast = false);
	UID CopyGO(const RE_GameObject* copy, UID parent, bool broadcast = false);
	UID CopyGOandChilds(const RE_GameObject* copy, UID parent, bool broadcast = false);
	UID InsertPool(RE_ECS_Pool* pool, bool broadcast = false);

	void DestroyGO(UID toDestroy);

	// Root
	UID GetRootUID() const;
	RE_GameObject* GetRootPtr() const;
	const RE_GameObject* GetRootCPtr() const;

	// GO Getters
	RE_GameObject* GetGOPtr(UID id) const;
	const RE_GameObject* GetGOCPtr(UID id) const;

	eastl::vector<UID> GetAllGOUIDs() const;
	eastl::vector<RE_GameObject*> GetAllGOPtrs() const;
	eastl::vector<eastl::pair<const UID, RE_GameObject*>> GetAllGOData() const;

	unsigned int TotalGameObjects() const { return gameObjectsPool.GetCount(); };

	// Component Getters
	eastl::vector<UID> GetAllCompUID(ushortint type = 0) const;
	eastl::vector<RE_Component*> GetAllCompPtr(ushortint type = 0) const;
	eastl::vector<const RE_Component*> GetAllCompCPtr(ushortint type = 0) const;
	eastl::vector<eastl::pair<const UID, RE_Component*>> GetAllCompData(ushortint type = 0) const;

	RE_Component* GetComponentPtr(const UID poolid, ComponentType cType);
	const RE_Component* GetComponentCPtr(const UID poolid, ComponentType cType) const;

	RE_ECS_Pool* GetNewPoolFromID(UID id);

	// Resources
	eastl::vector<const char*> GetAllResources();
	void UseResources();
	void UnUseResources();

	// Serialization
	unsigned int GetBinarySize() const;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources);
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources);

	void SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources);
	void DeserializeJson(RE_Json* node, eastl::map<int, const char* >* resources);

private:

	GameObjectsPool gameObjectsPool;
	ComponentsPool componentsPool;
};