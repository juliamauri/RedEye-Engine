#pragma once

#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_CompCamera.h"
#include "RE_CompMesh.h"
#include "RE_CompPrimitive.h"

#include <EASTL/vector.h>
#include "PoolMapped.h"

class GameObjectManager : public PoolMapped<RE_GameObject, int> {
public:
	GameObjectManager() { }
	~GameObjectManager() { }

	void Clear();

	int Push(RE_GameObject val)override;
};


template<class COMPCLASS>
class ComponentPool : public PoolMapped<COMPCLASS, int>
{
public:
	ComponentPool() { }
	~ComponentPool() { }

	void Clear() {
		poolmapped_.clear();
		lastAvaibleIndex = 0;
	}

	int Push(COMPCLASS val)override
	{
		int ret = lastAvaibleIndex;
		val.SetPoolID(ret);
		PoolMapped::Push(val, ret);
		return ret;
	}

	int GetNewComponent() {
		return Push({});
	}
};

//Components Pools
typedef ComponentPool<RE_CompTransform> TransformsPool;
typedef ComponentPool<RE_CompCamera> CamerasPool;
typedef ComponentPool<RE_CompMesh> MeshesPool;
//Primitives
typedef ComponentPool<RE_CompRock> PRockPool;
typedef ComponentPool<RE_CompCube> PCubePool;
typedef ComponentPool<RE_CompDodecahedron> PDodecahedronPool;
typedef ComponentPool<RE_CompTetrahedron> PTetrahedronPool;
typedef ComponentPool<RE_CompOctohedron> POctohedronPool;
typedef ComponentPool<RE_CompIcosahedron> PIcosahedronPool;
typedef ComponentPool<RE_CompPlane> PPlanePool;
typedef ComponentPool<RE_CompSphere> PSpherePool;
typedef ComponentPool<RE_CompCylinder> PCylinderPool;
typedef ComponentPool<RE_CompHemiSphere> PHemiSpherePool;
typedef ComponentPool<RE_CompTorus> PTorusPool;
typedef ComponentPool<RE_CompTrefoiKnot> PTrefoiKnotPool;


class ComponentsPool {
public:
	ComponentsPool() { }
	~ComponentsPool() { }

	void ClearComponents();

	RE_Component* GetComponent(int poolid, ComponentType cType);
	RE_Component* GetNewComponent(ComponentType cType);
	RE_Component* CopyComponent(RE_Component* cmp, RE_GameObject* parent);

	void DeleteTransform(int id);

private:
	CamerasPool camPool;
	TransformsPool transPool;
	MeshesPool meshPool;
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

class RE_GOManager
{
public:
	RE_GOManager();
	~RE_GOManager();

	RE_GameObject* AddGO(const char* name, RE_GameObject* parent);
	RE_GameObject* AddGO(const char* name, UUID uuid, RE_GameObject* parent);
	RE_GameObject* CopyGO(RE_GameObject* copy, RE_GameObject* parent);
	RE_GameObject* GetGO(int id)const;
	eastl::vector<int> GetAllGOs();
	unsigned int TotalGameObjects()const { return gameObjectsPool.GetCount(); };

	RE_GameObject* InsertPool(RE_GOManager* pool);

	RE_GOManager* GetNewPoolFromID(int id);

	void ClearPool();
	
private:
	RE_GameObject* RepercusiveInsertGO(RE_GameObject* go, RE_GameObject* parent);

private:
	GameObjectManager gameObjectsPool;
	ComponentsPool componentsPool;
};