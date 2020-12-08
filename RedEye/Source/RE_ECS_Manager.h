#pragma once

#include "PoolMapped.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_CompCamera.h"
#include "RE_CompMesh.h"
#include "RE_CompLight.h"
#include "RE_CompWater.h"
#include "RE_CompPrimitive.h"
#include "RE_FileSystem.h"

class JSONNode;
class GameObjectsPool;

template<class COMPCLASS, unsigned int size, unsigned int increment>
class ComponentPool : public PoolMapped<COMPCLASS, UID, size, increment>
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
		PoolMapped::Push(val, ret);
		PoolMapped::pool_[PoolMapped::poolmapped_.at(ret)].SetPoolID(ret);
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
		unsigned int cmpSize = GetCount();
		size += sizeof(UID) * cmpSize;
		for (unsigned int i = 0; i < cmpSize; i++) size += pool_[i].GetBinarySize();
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

	eastl::vector<UID> GetAllKeys() const override {
		eastl::vector<UID> ret;
		for (auto cmp : poolmapped_) ret.push_back(cmp.first);
		return ret;
	}

private:
	eastl::string cName;
};

//Components Pools
typedef ComponentPool<RE_CompTransform, 1024, 512> TransformsPool;
typedef ComponentPool<RE_CompCamera, 128, 64> CamerasPool;
typedef ComponentPool<RE_CompMesh, 128, 64> MeshesPool;
typedef ComponentPool<RE_CompLight, 128, 64> LightPool;
typedef ComponentPool<RE_CompWater, 8, 8> WaterPool;
//Primitives
typedef ComponentPool<RE_CompGrid, 128, 64> GridPool;
typedef ComponentPool<RE_CompRock, 128, 64> RockPool;
typedef ComponentPool<RE_CompCube, 128, 64> CubePool;
typedef ComponentPool<RE_CompDodecahedron, 128, 64> DodecahedronPool;
typedef ComponentPool<RE_CompTetrahedron, 128, 64> TetrahedronPool;
typedef ComponentPool<RE_CompOctohedron, 128, 64> OctohedronPool;
typedef ComponentPool<RE_CompIcosahedron, 128, 64> IcosahedronPool;
typedef ComponentPool<RE_CompPlane, 128, 64> PlanePool;
typedef ComponentPool<RE_CompSphere, 128, 64> SpherePool;
typedef ComponentPool<RE_CompCylinder, 128, 64> CylinderPool;
typedef ComponentPool<RE_CompHemiSphere, 128, 64> HemiSpherePool;
typedef ComponentPool<RE_CompTorus, 128, 64> TorusPool;
typedef ComponentPool<RE_CompTrefoiKnot, 128, 64> TrefoiKnotPool;

class ComponentsPool {
public:
	ComponentsPool() { 
		transPool.SetName("Transforms Pool");
		camPool.SetName("Cameras Pool");
		meshPool.SetName("Meshes Pool");
		lightPool.SetName("Lights Pool");
		waterPool.SetName("Water Pool");
		pGridPool.SetName("Grid Pool");
		pRockPool.SetName("Rock Pool");
		pDodecahedronPool.SetName("Dodecahedron Pool");
		pTetrahedronPool.SetName("Tetrahedron Pool");
		pOctohedronPool.SetName("Octohedron Pool");
		pIcosahedronPool.SetName("Icosahedron Pool");
		pPlanePool.SetName("Plane Pool");
		pSpherePool.SetName("Sphere Pool");
		pCylinderPool.SetName("Cylinder Pool");
		pHemiSpherePool.SetName("HemiSphere Pool");
		pTorusPool.SetName("orus Pool");
		pTrefoiKnotPool.SetName("TrefoiKnot Pool");
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
	eastl::pair<const UID, RE_Component*> GetNewComponent(ComponentType cType);
	const UID GetNewComponentUID(ComponentType cType);
	RE_Component* GetNewComponentPtr(ComponentType cType);

	RE_Component* CopyComponent(GameObjectsPool* pool, RE_Component* copy, const UID parent);
	void DestroyComponent(ComponentType cType, UID toDelete);

	// Component Getters
	RE_Component* GetComponentPtr(UID poolid, ComponentType cType);
	const RE_Component* GetComponentCPtr(UID poolid, ComponentType cType) const;

	eastl::vector<UID> GetAllCompUID(ushortint type = 0) const;
	eastl::vector<RE_Component*> GetAllCompPtr(ushortint type = 0) const;
	eastl::vector<const RE_Component*> GetAllCompCPtr(ushortint type = 0) const;
	eastl::vector<eastl::pair<const UID, RE_Component*>> GetAllCompData(ushortint type = 0) const;

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
	WaterPool waterPool;
	//Primitives
	GridPool pGridPool;
	RockPool pRockPool;
	CubePool pCubePool;
	DodecahedronPool pDodecahedronPool;
	TetrahedronPool pTetrahedronPool;
	OctohedronPool pOctohedronPool;
	IcosahedronPool pIcosahedronPool;
	PlanePool pPlanePool;
	SpherePool pSpherePool;
	CylinderPool pCylinderPool;
	HemiSpherePool pHemiSpherePool;
	TorusPool pTorusPool;
	TrefoiKnotPool pTrefoiKnotPool;
};

class GameObjectsPool : public PoolMapped<RE_GameObject, UID, 1024, 512> {
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
	eastl::vector<eastl::pair<const UID, RE_GameObject*>> GetAllData() const;

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

	eastl::vector<UID> GetAllKeys() const override;

private:

	UID Push(RE_GameObject val) override;
};

class RE_ECS_Manager
{
public:
	RE_ECS_Manager();
	~RE_ECS_Manager();

	// Content iteration
	void Update();
	void ClearPool();

	// Pool handling
	RE_GameObject* AddGO(const char* name, UID parent, bool broadcast = false);
	UID CopyGO(const RE_GameObject* copy, UID parent, bool broadcast = false);
	UID CopyGOandChilds(const RE_GameObject* copy, UID parent, bool broadcast = false);
	UID InsertPool(RE_ECS_Manager* pool, bool broadcast = false);

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

	RE_ECS_Manager* GetNewPoolFromID(UID id);


	// Resources
	eastl::vector<const char*> GetAllResources();
	void UseResources();
	void UnUseResources();

	// Serialization
	unsigned int GetBinarySize() const;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources);
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources);

	void SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources);
	void DeserializeJson(JSONNode* node, eastl::map<int, const char* >* resources);

private:
	GameObjectsPool gameObjectsPool;
	ComponentsPool componentsPool;
};