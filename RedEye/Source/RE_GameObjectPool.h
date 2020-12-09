#ifndef __RE_GAMEOBJECT_POOL_H__
#define __RE_GAMEOBJECT_POOL_H__

#include "RE_HashMap.h"
#include "RE_GameObject.h"

class GameObjectsPool : public RE_HashMap<RE_GameObject, UID, 1024, 512>
{
public:
	GameObjectsPool() {}
	~GameObjectsPool() {}

	// Content iteration
	void Clear();

	// Pool handling
	UID GetNewGOUID();
	RE_GameObject* GetNewGOPtr();
	void DeleteGO(UID toDelete);

	// Getters
	eastl::vector<RE_GameObject*> GetAllPtrs() const;
	eastl::vector<eastl::pair<const UID, RE_GameObject*>> GetAllData() const;

	// Root
	UID GetRootUID() const;
	RE_GameObject* GetRootPtr() const;
	const RE_GameObject* GetRootCPtr() const;

	// Serialization
	unsigned int GetBinarySize() const;
	void SerializeBinary(char*& cursor);
	void DeserializeBinary(char*& cursor, ComponentsPool* cmpsPool);

	void SerializeJson(RE_Json* node);
	void DeserializeJson(RE_Json* node, ComponentsPool* cmpsPool);

	eastl::vector<UID> GetAllKeys() const override;

private:

	UID Push(RE_GameObject val) override;
};

#endif // !__RE_GAMEOBJECT_POOL_H__