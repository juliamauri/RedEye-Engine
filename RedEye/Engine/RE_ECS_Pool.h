#ifndef __RE_ECS_POOL__
#define __RE_ECS_POOL__

#include "RE_ComponentsPool.h"
#include "RE_GameObjectPool.h"

class RE_ECS_Pool
{
public:
	RE_ECS_Pool() {}
	~RE_ECS_Pool() {}

	// Content iteration
	void Update();
	void ClearPool();

	// Pool handling
	RE_GameObject* AddGO(const char* name, GO_UID parent, bool broadcast = false);
	GO_UID CopyGO(const RE_GameObject* copy, GO_UID parent, bool broadcast = false);
	GO_UID CopyGOandChilds(const RE_GameObject* copy, GO_UID parent, bool broadcast = false);
	GO_UID InsertPool(RE_ECS_Pool* pool, bool broadcast = false);

	void DestroyGO(GO_UID toDestroy);

	// Root
	GO_UID GetRootUID() const;
	RE_GameObject* GetRootPtr() const;
	const RE_GameObject* GetRootCPtr() const;

	// GO Getters
	RE_GameObject* GetGOPtr(GO_UID id) const;
	const RE_GameObject* GetGOCPtr(GO_UID id) const;

	eastl::vector<GO_UID> GetAllGOUIDs() const;
	eastl::vector<RE_GameObject*> GetAllGOPtrs() const;
	eastl::vector<eastl::pair<const GO_UID, RE_GameObject*>> GetAllGOData() const;

	unsigned int TotalGameObjects() const { return gameObjectsPool.GetCount(); };

	RE_ECS_Pool* GetNewPoolFromID(GO_UID id);

	#pragma region Component Getters

	RE_Component* GetComponentPtr(const COMP_UID poolid, ComponentType cType);
	const RE_Component* GetComponentCPtr(const COMP_UID poolid, ComponentType cType) const;

	eastl::vector<COMP_UID> GetAllCompUID(ushort type = 0) const;
	eastl::vector<RE_Component*> GetAllCompPtr(ushort type = 0) const;
	eastl::vector<const RE_Component*> GetAllCompCPtr(ushort type = 0) const;
	eastl::vector<eastl::pair<const COMP_UID, RE_Component*>> GetAllCompData(ushort type = 0) const;

	#pragma endregion

	#pragma region Resources

	eastl::vector<const char*> GetAllResources();
	void UseResources();
	void UnUseResources();

	#pragma endregion

	#pragma region Serialization

	unsigned int GetBinarySize() const;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources);
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources);
	void SerializeJson(class RE_Json* node, eastl::map<const char*, int>* resources);
	void DeserializeJson(RE_Json* node, eastl::map<int, const char* >* resources);

	#pragma endregion

private:

	GameObjectsPool gameObjectsPool;
	ComponentsPool componentsPool;
};

#endif // !__RE_ECS_POOL__