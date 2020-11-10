#pragma once

#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_CompCamera.h"
#include "RE_CompMesh.h"
#include "RE_CompLight.h"
#include "RE_CompPrimitive.h"
#include "RE_FileSystem.h"

#include <EASTL/vector.h>
#include "PoolMapped.h"

class JSONNode;
class GameObjectsPool;

template<class COMPCLASS, unsigned int size>
class ComponentPool : public PoolMapped<COMPCLASS, UID, size>
{
public:
	ComponentPool() { }
	~ComponentPool() { }

	void SetName(const char* name) {
		cName = name;
	}

	void Update()
	{
		for (int i = 0; i < lastAvaibleIndex; ++i) pool_[i].Update();
	}

	void Clear() {
		poolmapped_.clear();
		lastAvaibleIndex = 0;
	}

	UID Push(COMPCLASS val)override
	{
		UID ret = RE_Math::RandomUID();
		val.SetPoolID(ret);
		PoolMapped::Push(val, ret);
		return ret;
	}

	UID GetNewCompUID() {
		return Push({});
	}

	eastl::vector<const char*> GetAllResources() {
		eastl::vector<const char*> ret;
		for (int i = 0; i < lastAvaibleIndex; i++) {
			eastl::vector<const char*> cmpres = pool_[i].GetAllResources();
			if (!cmpres.empty()) ret.insert(ret.end(), cmpres.begin(), cmpres.end());
		}
		return ret;
	}

	void UseResources() {
		for (int i = 0; i < lastAvaibleIndex; i++) 
			pool_[i].UseResources();
	}

	void UnUseResources() {
		for (int i = 0; i < lastAvaibleIndex; i++) 
			pool_[i].UnUseResources();
	}

	void SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources) {

		JSONNode* compPool = node->PushJObject(cName.c_str());

		unsigned int cmpSize = GetCount();
		compPool->PushUInt("poolSize", cmpSize);

		for (uint i = 0; i < cmpSize; i++) {
			JSONNode* comp = compPool->PushJObject(eastl::to_string(i).c_str());
			comp->PushUnsignedLongLong("parentPoolID", pool_[i].GetGOUID());
			pool_[i].SerializeJson(comp, resources);
			DEL(comp);
		}
		DEL(compPool);
	}

	void DeserializeJson(GameObjectsPool* goPool, JSONNode* node, eastl::map<int, const char*>* resources)
	{
		JSONNode* comp_objs = node->PullJObject(cName.c_str());
		unsigned int cmpSize = comp_objs->PullUInt("poolSize", 0);
		for (unsigned int i = 0; i < cmpSize; i++)
		{
			COMPCLASS* comp_ptr = AtPtr(Push({}));
			JSONNode* comp_obj = comp_objs->PullJObject(eastl::to_string(i).c_str());
			comp_ptr->PoolSetUp(goPool, comp_obj->PullUnsignedLongLong("parentPoolID", 0), true);
			comp_ptr->DeserializeJson(comp_obj, resources);

			DEL(comp_obj);
		}
		DEL(comp_objs);
	}

	unsigned int GetBinarySize()const {
		unsigned int size = sizeof(unsigned int);
		unsigned int totalComps = GetCount();
		if (totalComps > 0) size += (pool_[0].GetBinarySize() + sizeof(UID)) * totalComps;
		return size;
	}

	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) {
		unsigned int size = sizeof(unsigned int);
		unsigned int cmpSize = GetCount();

		memcpy(cursor, &cmpSize, size);
		cursor += size;

		size = sizeof(UID);
		for (unsigned int i = 0; i < cmpSize; i++)
		{
			UID goID = pool_[i].GetGOUID();
			memcpy(cursor, &goID, size);
			cursor += size;

			pool_[i].SerializeBinary(cursor, resources);
		}
	}

	void DeserializeBinary(GameObjectsPool* goPool, char*& cursor, eastl::map<int, const char*>* resources)
	{
		unsigned int size = sizeof(unsigned int);
		unsigned int totalComps;

		memcpy(&totalComps, cursor, size);
		cursor += size;

		size = sizeof(UID);
		for (uint i = 0; i < totalComps; i++)
		{
			UID goID;
			memcpy(&goID, cursor, size);
			cursor += size;

			COMPCLASS* comp_ptr = AtPtr(Push({}));
			comp_ptr->PoolSetUp(goPool, goID, true);
			comp_ptr->DeserializeBinary(cursor, resources);
		}
	}

private:
	eastl::string cName;
};

//Components Pools
typedef ComponentPool<RE_CompTransform, 10240> TransformsPool;
typedef ComponentPool<RE_CompCamera, 512> CamerasPool;
typedef ComponentPool<RE_CompMesh, 2048> MeshesPool;
typedef ComponentPool<RE_CompLight, 124> LightPool;
//Primitives
typedef ComponentPool<RE_CompRock, 1024> PRockPool;
typedef ComponentPool<RE_CompCube, 1024> PCubePool;
typedef ComponentPool<RE_CompDodecahedron, 1024> PDodecahedronPool;
typedef ComponentPool<RE_CompTetrahedron, 1024> PTetrahedronPool;
typedef ComponentPool<RE_CompOctohedron, 1024> POctohedronPool;
typedef ComponentPool<RE_CompIcosahedron, 1024> PIcosahedronPool;
typedef ComponentPool<RE_CompPlane, 1024> PPlanePool;
typedef ComponentPool<RE_CompSphere, 1024> PSpherePool;
typedef ComponentPool<RE_CompCylinder, 1024> PCylinderPool;
typedef ComponentPool<RE_CompHemiSphere, 1024> PHemiSpherePool;
typedef ComponentPool<RE_CompTorus, 1024> PTorusPool;
typedef ComponentPool<RE_CompTrefoiKnot, 1024> PTrefoiKnotPool;

class ComponentsPool {
public:
	ComponentsPool() { 
		transPool.SetName("Transforms Pool");
		camPool.SetName("Cameras Pool");
		meshPool.SetName("Meshes Pool");
		lightPool.SetName("Lights Pool");
		pRockPool.SetName("PRocks Pool");
		pCubePool.SetName("PCubes Pool");
		pDodecahedronPool.SetName("PDodecahedrons Pool");
		pTetrahedronPool.SetName("PTetrahedrons Pool");
		pOctohedronPool.SetName("POctohedron Pool");
		pIcosahedronPool.SetName("PIcosahedron Pool");
		pPlanePool.SetName("PPlane Pool");
		pSpherePool.SetName("PSphere Pool");
		pCylinderPool.SetName("PCylinder Pool");
		pHemiSpherePool.SetName("PHemiSphere Pool");
		pTorusPool.SetName("PTorus Pool");
		pTrefoiKnotPool.SetName("PTrefoiKnot Pool");
	}
	~ComponentsPool() { }

	// Content iteration
	void Update();
	void ClearComponents();

	// Resources
	eastl::vector<const char*> GetAllResources();
	void UseResources();
	void UnUseResources();

	// Component Handling
	eastl::pair<UID, RE_Component*> GetNewComponent(ComponentType cType);
	UID GetNewComponentUID(ComponentType cType);
	RE_Component* GetNewComponentPtr(ComponentType cType);

	RE_Component* CopyComponent(GameObjectsPool* pool, RE_Component* copy, const UID parent);
	void DestroyComponent(ComponentType cType, UID toDelete);

	// Component Getters
	RE_Component* GetComponentPtr(UID poolid, ComponentType cType);
	const RE_Component* GetComponentCPtr(UID poolid, ComponentType cType);

	eastl::vector<UID> GetAllCompUID(ushortint type = 0) const;
	eastl::vector<RE_Component*> GetAllCompPtr(ushortint type = 0) const;
	eastl::vector<RE_Component*> GetAllCompCPtr(ushortint type = 0) const;
	eastl::vector<eastl::pair<UID, RE_Component*>> GetAllCompData(ushortint type = 0) const;

	// Serialization
	unsigned int GetBinarySize()const;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources);
	void DeserializeBinary(GameObjectsPool* goPool, char*& cursor, eastl::map<int, const char*>* resources);

	void SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources);
	void DeserializeJson(GameObjectsPool* goPool, JSONNode* node, eastl::map<int, const char*>* resources);

private:
	TransformsPool transPool;
	CamerasPool camPool;
	MeshesPool meshPool;
	LightPool lightPool;
	PRockPool pRockPool;
	PCubePool pCubePool;
	PDodecahedronPool pDodecahedronPool;
	PTetrahedronPool pTetrahedronPool;
	POctohedronPool pOctohedronPool;
	PIcosahedronPool pIcosahedronPool;
	PPlanePool pPlanePool;
	PSpherePool pSpherePool;
	PCylinderPool pCylinderPool;
	PHemiSpherePool pHemiSpherePool;
	PTorusPool pTorusPool;
	PTrefoiKnotPool pTrefoiKnotPool;
};

class GameObjectsPool : public PoolMapped<RE_GameObject, UID, 10240> {
public:
	GameObjectsPool() { }
	~GameObjectsPool() { }

	// Content iteration
	void Clear();

	// Pool handling
	UID GetNewGOUID();
	RE_GameObject* GetNewGOPtr();
	void DeleteGO(UID toDelete);

	// Getters
	eastl::vector<RE_GameObject*> GetAllPtrs() const;
	eastl::vector<eastl::pair<UID, RE_GameObject*>> GetAllData() const;

	// Root
	UID GetRootUID() const;
	RE_GameObject* GetRootPtr() const;
	const RE_GameObject* GetRootCPtr() const;

	// Serialization
	unsigned int GetBinarySize() const;
	void SerializeBinary(char*& cursor);
	void DeserializeBinary(char*& cursor, ComponentsPool* cmpsPool);

	void SerializeJson(JSONNode* node);
	void DeserializeJson(JSONNode* node, ComponentsPool* cmpsPool);

private:

	UID Push(RE_GameObject val) override;
};

class RE_GOManager
{
public:
	RE_GOManager();
	~RE_GOManager();

	// Content iteration
	void Update();
	void ClearPool();

	// Pool handling
	RE_GameObject* AddGO(const char* name, UID parent, bool broadcast = false);
	RE_GameObject* CopyGO(RE_GameObject* copy, UID parent, bool broadcast = false);
	RE_GameObject* InsertPool(RE_GOManager* pool);

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
	eastl::vector<eastl::pair<UID, RE_GameObject*>> GetAllGOData() const;

	unsigned int TotalGameObjects() const { return gameObjectsPool.GetCount(); };

	// Component Getters
	eastl::vector<UID> GetAllCompUID(ushortint type = 0) const;
	eastl::vector<RE_Component*> GetAllCompPtr(ushortint type = 0) const;
	eastl::vector<RE_Component*> GetAllCompCPtr(ushortint type = 0) const;
	eastl::vector<eastl::pair<UID, RE_Component*>> GetAllCompData(ushortint type = 0) const;
	RE_GOManager* GetNewPoolFromID(UID id);

	// Resources
	eastl::vector<const char*> GetAllResources();
	void UseResources();
	void UnUseResources();

	// Serialization
	unsigned int GetBinarySize()const;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources);
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources);

	void SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources);
	void DeserializeJson(JSONNode* node, eastl::map<int, const char* >* resources);
	
private:

	RE_GameObject* RecusiveInsertGO(RE_GameObject* go, UID parent);
	void RecursiveDestroyGO(UID toDestroy);

private:
	GameObjectsPool gameObjectsPool;
	ComponentsPool componentsPool;
};