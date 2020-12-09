#ifndef __RE_COMPONENTS_POOL_H__
#define __RE_COMPONENTS_POOL_H__

#include "RE_ComponentPool.h"

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

	void SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources);
	void DeserializeJson(GameObjectsPool* goPool, RE_Json* node, eastl::map<int, const char*>* resources);

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
#endif // !__RE_COMPONENTS_POOL_H__