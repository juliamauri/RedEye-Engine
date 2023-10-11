#ifndef __RE_COMPONENTS_POOL_H__
#define __RE_COMPONENTS_POOL_H__

#include "RE_ComponentPool.h"

class ComponentsPool
{
public:
	ComponentsPool();
	~ComponentsPool() = default;

	// Content iteration
	void Update();
	void ClearComponents();

	// Resources
	eastl::vector<const char*> GetAllResources();
	void UseResources();
	void UnUseResources();

	// Component Handling
	eastl::pair<const COMP_UID, RE_Component*> GetNewComponent(RE_Component::Type cType);
	COMP_UID GetNewComponentUID(RE_Component::Type cType);
	RE_Component* GetNewComponentPtr(RE_Component::Type cType);

	RE_Component* CopyComponent(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent);
	void DestroyComponent(RE_Component::Type cType, COMP_UID toDelete);

	// Component Getters
	RE_Component* GetComponentPtr(COMP_UID poolid, RE_Component::Type cType);
	const RE_Component* GetComponentCPtr(COMP_UID poolid, RE_Component::Type cType) const;

	eastl::vector<COMP_UID> GetAllCompUID(RE_Component::Type type = RE_Component::Type::EMPTY) const;
	eastl::vector<RE_Component*> GetAllCompPtr(RE_Component::Type type = RE_Component::Type::EMPTY) const;
	eastl::vector<const RE_Component*> GetAllCompCPtr(RE_Component::Type type = RE_Component::Type::EMPTY) const;
	eastl::vector<eastl::pair<const COMP_UID, RE_Component*>> GetAllCompData(RE_Component::Type type = RE_Component::Type::EMPTY) const;
	eastl::vector<eastl::pair<const COMP_UID, const RE_Component*>> GetAllCompCData(RE_Component::Type type = RE_Component::Type::EMPTY) const;

	// Serialization
	void SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources);
	void DeserializeJson(GameObjectsPool* goPool, RE_Json* node, eastl::map<int, const char*>* resources);

	size_t GetBinarySize() const;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources);
	void DeserializeBinary(GameObjectsPool* goPool, char*& cursor, eastl::map<int, const char*>* resources);

private:

	TransformsPool transPool;
	CamerasPool camPool;
	MeshesPool meshPool;
	LightPool lightPool;
	WaterPool waterPool;
	ParticleSystemPool particleSPool;

	//Primitives
	GridPool pGridPool;
	RockPool pRockPool;
	CubePool pCubePool;
	DodecahedronPool pDodecahedronPool;
	TetrahedronPool pTetrahedronPool;
	OctohedronPool pOctohedronPool;
	IcosahedronPool pIcosahedronPool;
	PointPool pPointPool;
	PlanePool pPlanePool;
	SpherePool pSpherePool;
	CylinderPool pCylinderPool;
	HemiSpherePool pHemiSpherePool;
	TorusPool pTorusPool;
	TrefoiKnotPool pTrefoiKnotPool;
};
#endif // !__RE_COMPONENTS_POOL_H__