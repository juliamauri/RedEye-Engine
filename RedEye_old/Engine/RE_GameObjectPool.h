#ifndef __RE_GAMEOBJECT_POOL_H__
#define __RE_GAMEOBJECT_POOL_H__

#include "RE_HashMap.h"
#include "RE_GameObject.h"

class GameObjectsPool : public RE_HashMap<RE_GameObject, GO_UID, 1024, 512>
{
public:
	GameObjectsPool() {}
	~GameObjectsPool() {}

	// Content iteration
	void Clear();

	// Pool handling
	GO_UID GetNewGOUID();
	RE_GameObject* GetNewGOPtr();
	void DeleteGO(GO_UID toDelete);

	// Getters
	eastl::vector<RE_GameObject*> GetAllPtrs() const;
	eastl::vector<eastl::pair<const GO_UID, RE_GameObject*>> GetAllData() const;
	eastl::vector<eastl::pair<const GO_UID, const RE_GameObject*>> GetAllCData() const;

	// Root
	GO_UID GetRootUID() const;
	RE_GameObject* GetRootPtr() const;
	const RE_GameObject* GetRootCPtr() const;

	// Serialization
	size_t GetBinarySize() const;
	void SerializeBinary(char*& cursor);
	void DeserializeBinary(char*& cursor, ComponentsPool* cmpsPool);

	void SerializeJson(RE_Json* node);
	void DeserializeJson(RE_Json* node, ComponentsPool* cmpsPool);

	eastl::vector<GO_UID> GetAllKeys() const override;

private:

	GO_UID Push(RE_GameObject val) override;
};

#endif // !__RE_GAMEOBJECT_POOL_H__