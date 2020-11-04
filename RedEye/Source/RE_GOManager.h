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
class GameObjectManager;

template<class COMPCLASS, unsigned int size>
class ComponentPool : public PoolMapped<COMPCLASS, UID, size>
{
public:
	ComponentPool() { }
	~ComponentPool() { }

	void SetName(const char* name) {
		cName = name;
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

	UID GetNewComponent() {
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
			comp->PushUnsignedLongLong("parentPoolID", pool_[i].GetGO()->GetPoolID());
			pool_[i].SerializeJson(comp, resources);
			DEL(comp);
		}
		DEL(compPool);
	}

	void DeserializeJson(GameObjectManager* goPool, JSONNode* node, eastl::map<int, const char*>* resources) {

		JSONNode* compPool = node->PullJObject(cName.c_str());

		unsigned int cmpSize = compPool->PullUInt("poolSize", 0);

		for (unsigned int i = 0; i < cmpSize; i++) {
			JSONNode* comp = compPool->PullJObject(eastl::to_string(i).c_str());

			RE_GameObject* cParent = goPool->AtPtr(compPool->PullUnsignedLongLong("parentPoolID", 0));
			AtPtr(Push({}))->DeserializeJson(comp, resources, cParent);

			DEL(comp);
		}
		DEL(compPool);
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
		for (unsigned int i = 0; i < cmpSize; i++) {
			UID goID = pool_[i].GetGO()->GetPoolID();
			memcpy(cursor, &goID, size);
			cursor += size;

			pool_[i].SerializeBinary(cursor, resources);
		}
	}

	void DeserializeBinary(GameObjectManager* goPool, char*& cursor, eastl::map<int, const char*>* resources) {
		unsigned int size = sizeof(unsigned int);
		unsigned int totalComps;

		memcpy(&totalComps, cursor, size);
		cursor += size;

		size = sizeof(UID);
		for (uint i = 0; i < totalComps; i++) {
			UID goID;
			memcpy(&goID, cursor, size);
			cursor += size;
			AtPtr(Push({}))->DeserializeBinary(cursor, resources, goPool->AtPtr(goID));
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

	void ClearComponents();

	RE_Component* GetComponent(UID poolid, ComponentType cType);
	RE_Component* GetNewComponent(ComponentType cType);
	RE_Component* CopyComponent(RE_Component* cmp, RE_GameObject* parent);

	void DestroyComponent(ComponentType cType, UID toDelete);

	eastl::vector<const char*> GetAllResources();
	void UseResources();
	void UnUseResources();

	eastl::stack<RE_CompLight*> GetAllLights(bool check_active);

	unsigned int GetBinarySize()const;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources);
	void DeserializeBinary(GameObjectManager* goPool, char*& cursor, eastl::map<int, const char*>* resources);

	void SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources);
	void DeserializeJson(GameObjectManager* goPool, JSONNode* node, eastl::map<int, const char*>* resources);

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

class GameObjectManager : public PoolMapped<RE_GameObject, UID, 10240> {
public:
	GameObjectManager() { }
	~GameObjectManager() { }

	void Clear();

	UID Push(RE_GameObject val)override;
	UID GetFirstGOUID();
	RE_GameObject* GetFirstGO();

	void DeleteGO(UID toDelete);

	unsigned int GetBinarySize()const;
	void SerializeBinary(char*& cursor);
	void DeserializeBinary(char*& cursor, ComponentsPool* cmpsPool);

	void SerializeJson(JSONNode* node);
	void DeserializeJson(JSONNode* node, ComponentsPool* cmpsPool);
};

class RE_GOManager
{
public:
	RE_GOManager();
	~RE_GOManager();

	RE_GameObject* AddGO(const char* name, RE_GameObject* parent);
	RE_GameObject* CopyGO(RE_GameObject* copy, RE_GameObject* parent);
	RE_GameObject* GetGO(UID id)const;
	UID GetFirstGOUID();
	RE_GameObject* GetFirstGO();
	eastl::vector<UID> GetAllGOs();
	unsigned int TotalGameObjects()const { return gameObjectsPool.GetCount(); };

	void DestroyGO(UID toDestroy);

	RE_GameObject* InsertPool(RE_GOManager* pool);

	RE_GOManager* GetNewPoolFromID(UID id);

	eastl::stack<RE_CompLight*> GetAllLights(bool check_active);

	eastl::vector<const char*> GetAllResources();
	void UseResources();
	void UnUseResources();

	void ClearPool();

	unsigned int GetBinarySize()const;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources);
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources);

	void SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources);
	void DeserializeJson(JSONNode* node, eastl::map<int, const char* >* resources);
	
private:
	RE_GameObject* RepercusiveInsertGO(RE_GameObject* go, RE_GameObject* parent);

private:
	GameObjectManager gameObjectsPool;
	ComponentsPool componentsPool;
};