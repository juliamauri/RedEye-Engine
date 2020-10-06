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

	RE_CompTransform* GetNewTransform();
	RE_CompTransform* CopyTransform(RE_CompTransform* camComp, RE_GameObject* parent);
	void DeleteTransform(int id);

	RE_CompCamera* GetNewCamera();
	RE_CompCamera* CopyCamera(RE_CompCamera* camComp, RE_GameObject* parent);

	RE_CompMesh* GetNewMesh();
	RE_CompMesh* CopyMesh(RE_CompMesh* camComp, RE_GameObject* parent);

private:
	CamerasPool camPool;
	TransformsPool transPool;
	MeshesPool meshPool;
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