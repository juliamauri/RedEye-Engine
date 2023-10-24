#include "RE_ComponentsPool.h"

#include "RE_Memory.h"
#include "RE_Json.h"
#include "RE_GameObject.h"

ComponentsPool::ComponentsPool()
{
	transPool.SetName("Transforms Pool");
	camPool.SetName("Cameras Pool");
	meshPool.SetName("Meshes Pool");
	lightPool.SetName("Lights Pool");
	waterPool.SetName("Water Pool");
	particleSPool.SetName("Particle System Pool");
	pGridPool.SetName("Grid Pool");
	pRockPool.SetName("Rock Pool");
	pDodecahedronPool.SetName("Dodecahedron Pool");
	pTetrahedronPool.SetName("Tetrahedron Pool");
	pOctohedronPool.SetName("Octohedron Pool");
	pIcosahedronPool.SetName("Icosahedron Pool");
	pPointPool.SetName("Point Pool");
	pPlanePool.SetName("Plane Pool");
	pSpherePool.SetName("Sphere Pool");
	pCylinderPool.SetName("Cylinder Pool");
	pHemiSpherePool.SetName("HemiSphere Pool");
	pTorusPool.SetName("orus Pool");
	pTrefoiKnotPool.SetName("TrefoiKnot Pool");
}

void ComponentsPool::Update()
{
	transPool.Update();
	camPool.Update();
	particleSPool.Update();
}

void ComponentsPool::ClearComponents()
{
	transPool.Clear();
	camPool.Clear();
	meshPool.Clear();
	lightPool.Clear();
	waterPool.Clear();
	particleSPool.Clear();
	pGridPool.Clear();
	pRockPool.Clear();
	pCubePool.Clear();
	pDodecahedronPool.Clear();
	pTetrahedronPool.Clear();
	pOctohedronPool.Clear();
	pIcosahedronPool.Clear();
	pPointPool.Clear();
	pPlanePool.Clear();
	pSpherePool.Clear();
	pCylinderPool.Clear();
	pHemiSpherePool.Clear();
	pTorusPool.Clear();
	pTrefoiKnotPool.Clear();
}

eastl::vector<const char*> ComponentsPool::GetAllResources()
{
	eastl::vector<const char*> allResources;
	eastl::vector<const char*> resources;

	resources = camPool.GetAllResources();
	if (!resources.empty())
		allResources.insert(allResources.end(), resources.begin(), resources.end());

	resources = meshPool.GetAllResources();
	if (!resources.empty()) allResources.insert(allResources.end(), resources.begin(), resources.end());

	resources = particleSPool.GetAllResources();
	if (!resources.empty()) allResources.insert(allResources.end(), resources.begin(), resources.end());

	eastl::vector<const char*> ret;
	size_t resSize = 0;
	for (auto res : allResources)
	{
		bool repeat = false;
		for (auto uniqueRes : ret)
		{
			resSize = eastl::CharStrlen(res);
			if (resSize > 0 && eastl::Compare(res, uniqueRes, resSize) == 0)
			{
				repeat = true;
				break;
			}
		}
		if (!repeat) ret.push_back(res);
	}

	return ret;
}

void ComponentsPool::UseResources()
{
	camPool.UseResources();
	meshPool.UseResources();
	waterPool.UseResources();
	particleSPool.UseResources();
}

void ComponentsPool::UnUseResources()
{
	camPool.UnUseResources();
	meshPool.UnUseResources();
	waterPool.UnUseResources();
	particleSPool.UnUseResources();
}

RE_Component* ComponentsPool::GetComponentPtr(COMP_UID poolid, RE_Component::Type cType) const
{
	RE_Component* ret = nullptr;

	switch (cType)
	{
	case RE_Component::Type::TRANSFORM: ret = static_cast<RE_Component*>(transPool.AtPtr(poolid)); break;
	case RE_Component::Type::CAMERA: ret = static_cast<RE_Component*>(camPool.AtPtr(poolid)); break;
	case RE_Component::Type::MESH: ret = static_cast<RE_Component*>(meshPool.AtPtr(poolid)); break;
	case RE_Component::Type::LIGHT: ret = static_cast<RE_Component*>(lightPool.AtPtr(poolid)); break;
	case RE_Component::Type::WATER: ret = static_cast<RE_Component*>(waterPool.AtPtr(poolid)); break;
	case RE_Component::Type::PARTICLEEMITER: ret = static_cast<RE_Component*>(particleSPool.AtPtr(poolid)); break;
	case RE_Component::Type::GRID: ret = static_cast<RE_Component*>(pGridPool.AtPtr(poolid)); break;
	case RE_Component::Type::ROCK: ret = static_cast<RE_Component*>(pRockPool.AtPtr(poolid)); break;
	case RE_Component::Type::CUBE: ret = static_cast<RE_Component*>(pCubePool.AtPtr(poolid)); break;
	case RE_Component::Type::DODECAHEDRON: ret = static_cast<RE_Component*>(pDodecahedronPool.AtPtr(poolid)); break;
	case RE_Component::Type::TETRAHEDRON: ret = static_cast<RE_Component*>(pTetrahedronPool.AtPtr(poolid)); break;
	case RE_Component::Type::OCTOHEDRON: ret = static_cast<RE_Component*>(pOctohedronPool.AtPtr(poolid)); break;
	case RE_Component::Type::ICOSAHEDRON: ret = static_cast<RE_Component*>(pIcosahedronPool.AtPtr(poolid)); break;
	case RE_Component::Type::POINT: ret = static_cast<RE_Component*>(pPointPool.AtPtr(poolid)); break;
	case RE_Component::Type::PLANE: ret = static_cast<RE_Component*>(pPlanePool.AtPtr(poolid)); break;
	case RE_Component::Type::SPHERE: ret = static_cast<RE_Component*>(pSpherePool.AtPtr(poolid)); break;
	case RE_Component::Type::CYLINDER: ret = static_cast<RE_Component*>(pCylinderPool.AtPtr(poolid)); break;
	case RE_Component::Type::HEMISHPERE: ret = static_cast<RE_Component*>(pHemiSpherePool.AtPtr(poolid)); break;
	case RE_Component::Type::TORUS: ret = static_cast<RE_Component*>(pTorusPool.AtPtr(poolid)); break;
	case RE_Component::Type::TREFOILKNOT:  ret = static_cast<RE_Component*>(pTrefoiKnotPool.AtPtr(poolid)); break;
	default: break;
	}

	return ret;
}

const RE_Component* ComponentsPool::GetComponentCPtr(COMP_UID poolid, RE_Component::Type cType) const
{
	const RE_Component* ret = nullptr;

	switch (cType) {
	case RE_Component::Type::TRANSFORM: ret = static_cast<const RE_Component*>(transPool.AtCPtr(poolid)); break;
	case RE_Component::Type::CAMERA: ret = static_cast<const RE_Component*>(camPool.AtCPtr(poolid)); break;
	case RE_Component::Type::MESH: ret = static_cast<const RE_Component*>(meshPool.AtCPtr(poolid)); break;
	case RE_Component::Type::LIGHT: ret = static_cast<const RE_Component*>(lightPool.AtCPtr(poolid)); break;
	case RE_Component::Type::WATER: ret = static_cast<const RE_Component*>(waterPool.AtCPtr(poolid)); break;
	case RE_Component::Type::PARTICLEEMITER: ret = static_cast<const RE_Component*>(particleSPool.AtCPtr(poolid)); break;
	case RE_Component::Type::FUSTRUM:  break;
	case RE_Component::Type::GRID: ret = static_cast<const RE_Component*>(pGridPool.AtCPtr(poolid)); break;
	case RE_Component::Type::ROCK: ret = static_cast<const RE_Component*>(pRockPool.AtCPtr(poolid)); break;
	case RE_Component::Type::CUBE: ret = static_cast<const RE_Component*>(pCubePool.AtCPtr(poolid)); break;
	case RE_Component::Type::DODECAHEDRON: ret = static_cast<const RE_Component*>(pDodecahedronPool.AtCPtr(poolid)); break;
	case RE_Component::Type::TETRAHEDRON: ret = static_cast<const RE_Component*>(pTetrahedronPool.AtCPtr(poolid)); break;
	case RE_Component::Type::OCTOHEDRON: ret = static_cast<const RE_Component*>(pOctohedronPool.AtCPtr(poolid)); break;
	case RE_Component::Type::ICOSAHEDRON: ret = static_cast<const RE_Component*>(pIcosahedronPool.AtCPtr(poolid)); break;
	case RE_Component::Type::POINT: ret = static_cast<const RE_Component*>(pPointPool.AtCPtr(poolid)); break;
	case RE_Component::Type::PLANE: ret = static_cast<const RE_Component*>(pPlanePool.AtCPtr(poolid)); break;
	case RE_Component::Type::SPHERE: ret = static_cast<const RE_Component*>(pSpherePool.AtCPtr(poolid)); break;
	case RE_Component::Type::CYLINDER: ret = static_cast<const RE_Component*>(pCylinderPool.AtCPtr(poolid)); break;
	case RE_Component::Type::HEMISHPERE: ret = static_cast<const RE_Component*>(pHemiSpherePool.AtCPtr(poolid)); break;
	case RE_Component::Type::TORUS: ret = static_cast<const RE_Component*>(pTorusPool.AtCPtr(poolid)); break;
	case RE_Component::Type::TREFOILKNOT: ret = static_cast<const RE_Component*>(pTrefoiKnotPool.AtCPtr(poolid)); break;
	default: break;
	}

	return ret;
}

eastl::pair<const COMP_UID, RE_Component*> ComponentsPool::GetNewComponent(RE_Component::Type cType)
{
	switch (cType)
	{
	case RE_Component::Type::TRANSFORM: { const COMP_UID id = transPool.GetNewCompUID(); return { id, transPool.AtPtr(id) }; }
	case RE_Component::Type::CAMERA: { const COMP_UID id = camPool.GetNewCompUID(); return { id, camPool.AtPtr(id) }; }
	case RE_Component::Type::MESH: { const COMP_UID id = meshPool.GetNewCompUID(); return { id, meshPool.AtPtr(id) }; }
	case RE_Component::Type::LIGHT: { const COMP_UID id = lightPool.GetNewCompUID(); return { id, lightPool.AtPtr(id) }; }
	case RE_Component::Type::WATER: { const COMP_UID id = waterPool.GetNewCompUID(); return { id, waterPool.AtPtr(id) }; }
	case RE_Component::Type::PARTICLEEMITER: { const COMP_UID id = particleSPool.GetNewCompUID(); return { id, particleSPool.AtPtr(id) }; }
	case RE_Component::Type::GRID: { const COMP_UID id = pGridPool.GetNewCompUID(); return { id, pGridPool.AtPtr(id) }; }
	case RE_Component::Type::ROCK: { const COMP_UID id = pRockPool.GetNewCompUID(); return { id, pRockPool.AtPtr(id) }; }
	case RE_Component::Type::CUBE: { const COMP_UID id = pCubePool.GetNewCompUID(); return { id, pCubePool.AtPtr(id) }; }
	case RE_Component::Type::DODECAHEDRON: { const COMP_UID id = pDodecahedronPool.GetNewCompUID(); return { id, pDodecahedronPool.AtPtr(id) }; }
	case RE_Component::Type::TETRAHEDRON: { const COMP_UID id = pTetrahedronPool.GetNewCompUID(); return { id, pTetrahedronPool.AtPtr(id) }; }
	case RE_Component::Type::OCTOHEDRON: { const COMP_UID id = pOctohedronPool.GetNewCompUID(); return { id, pOctohedronPool.AtPtr(id) }; }
	case RE_Component::Type::ICOSAHEDRON: { const COMP_UID id = pIcosahedronPool.GetNewCompUID(); return { id, pIcosahedronPool.AtPtr(id) }; }
	case RE_Component::Type::POINT: { const COMP_UID id = pPointPool.GetNewCompUID(); return { id, pPointPool.AtPtr(id) }; }
	case RE_Component::Type::PLANE: { const COMP_UID id = pPlanePool.GetNewCompUID(); return { id, pPlanePool.AtPtr(id) }; }
	case RE_Component::Type::SPHERE: { const COMP_UID id = pSpherePool.GetNewCompUID(); return { id, pSpherePool.AtPtr(id) }; }
	case RE_Component::Type::CYLINDER: { const COMP_UID id = pCylinderPool.GetNewCompUID(); return { id, pCylinderPool.AtPtr(id) }; }
	case RE_Component::Type::HEMISHPERE: { const COMP_UID id = pHemiSpherePool.GetNewCompUID(); return { id, pHemiSpherePool.AtPtr(id) }; }
	case RE_Component::Type::TORUS: { const COMP_UID id = pTorusPool.GetNewCompUID(); return { id, pTorusPool.AtPtr(id) }; }
	case RE_Component::Type::TREFOILKNOT: { const COMP_UID id = pTrefoiKnotPool.GetNewCompUID(); return { id, pTrefoiKnotPool.AtPtr(id) }; }
	default: return { 0 , nullptr };
	}
}

COMP_UID ComponentsPool::GetNewComponentUID(RE_Component::Type cType)
{
	switch (cType)
	{
	case RE_Component::Type::TRANSFORM: return transPool.GetNewCompUID();
	case RE_Component::Type::CAMERA: return camPool.GetNewCompUID();
	case RE_Component::Type::MESH: return meshPool.GetNewCompUID();
	case RE_Component::Type::LIGHT: return lightPool.GetNewCompUID();
	case RE_Component::Type::WATER: return waterPool.GetNewCompUID();
	case RE_Component::Type::PARTICLEEMITER: particleSPool.GetNewCompUID(); break;
	case RE_Component::Type::GRID: return pGridPool.GetNewCompUID();
	case RE_Component::Type::ROCK: return pRockPool.GetNewCompUID();
	case RE_Component::Type::CUBE: return pCubePool.GetNewCompUID();
	case RE_Component::Type::DODECAHEDRON: return pDodecahedronPool.GetNewCompUID();
	case RE_Component::Type::TETRAHEDRON: return pTetrahedronPool.GetNewCompUID();
	case RE_Component::Type::OCTOHEDRON: return pOctohedronPool.GetNewCompUID();
	case RE_Component::Type::ICOSAHEDRON:  return pIcosahedronPool.GetNewCompUID();
	case RE_Component::Type::POINT: return pPointPool.GetNewCompUID();
	case RE_Component::Type::PLANE: return pPlanePool.GetNewCompUID();
	case RE_Component::Type::SPHERE: return pSpherePool.GetNewCompUID();
	case RE_Component::Type::CYLINDER: return pCylinderPool.GetNewCompUID();
	case RE_Component::Type::HEMISHPERE: return pHemiSpherePool.GetNewCompUID();
	case RE_Component::Type::TORUS:  return pTorusPool.GetNewCompUID();
	case RE_Component::Type::TREFOILKNOT: return pTrefoiKnotPool.GetNewCompUID();
	default: break;
	}

	return 0;
}

RE_Component* ComponentsPool::GetNewComponentPtr(RE_Component::Type cType)
{
	switch (cType)
	{
	case RE_Component::Type::TRANSFORM: return static_cast<RE_Component*>(transPool.AtPtr(transPool.GetNewCompUID()));
	case RE_Component::Type::CAMERA: return static_cast<RE_Component*>(camPool.AtPtr(camPool.GetNewCompUID()));
	case RE_Component::Type::MESH: return static_cast<RE_Component*>(meshPool.AtPtr(meshPool.GetNewCompUID()));
	case RE_Component::Type::LIGHT: return static_cast<RE_Component*>(lightPool.AtPtr(lightPool.GetNewCompUID()));
	case RE_Component::Type::WATER: return static_cast<RE_Component*>(waterPool.AtPtr(waterPool.GetNewCompUID()));
	case RE_Component::Type::PARTICLEEMITER: return static_cast<RE_Component*>(particleSPool.AtPtr(particleSPool.GetNewCompUID()));
	case RE_Component::Type::GRID: return static_cast<RE_Component*>(pGridPool.AtPtr(pGridPool.GetNewCompUID()));
	case RE_Component::Type::ROCK: return static_cast<RE_Component*>(pRockPool.AtPtr(pRockPool.GetNewCompUID()));
	case RE_Component::Type::CUBE: return static_cast<RE_Component*>(pCubePool.AtPtr(pCubePool.GetNewCompUID()));
	case RE_Component::Type::DODECAHEDRON: return static_cast<RE_Component*>(pDodecahedronPool.AtPtr(pDodecahedronPool.GetNewCompUID()));
	case RE_Component::Type::TETRAHEDRON: return static_cast<RE_Component*>(pTetrahedronPool.AtPtr(pTetrahedronPool.GetNewCompUID()));
	case RE_Component::Type::OCTOHEDRON: return static_cast<RE_Component*>(pOctohedronPool.AtPtr(pOctohedronPool.GetNewCompUID()));
	case RE_Component::Type::ICOSAHEDRON: return static_cast<RE_Component*>(pIcosahedronPool.AtPtr(pIcosahedronPool.GetNewCompUID()));
	case RE_Component::Type::POINT: return static_cast<RE_Component*>(pPointPool.AtPtr(pPointPool.GetNewCompUID()));
	case RE_Component::Type::PLANE: return static_cast<RE_Component*>(pPlanePool.AtPtr(pPlanePool.GetNewCompUID()));
	case RE_Component::Type::SPHERE: return static_cast<RE_Component*>(pSpherePool.AtPtr(pSpherePool.GetNewCompUID()));
	case RE_Component::Type::CYLINDER: return static_cast<RE_Component*>(pCylinderPool.AtPtr(pCylinderPool.GetNewCompUID()));
	case RE_Component::Type::HEMISHPERE: return static_cast<RE_Component*>(pHemiSpherePool.AtPtr(pHemiSpherePool.GetNewCompUID()));
	case RE_Component::Type::TORUS: return static_cast<RE_Component*>(pTorusPool.AtPtr(pTorusPool.GetNewCompUID()));
	case RE_Component::Type::TREFOILKNOT: return static_cast<RE_Component*>(pTrefoiKnotPool.AtPtr(pTrefoiKnotPool.GetNewCompUID()));
	default: return nullptr;
	}
}

RE_Component* ComponentsPool::CopyComponent(GameObjectsPool* pool, RE_Component* copy, GO_UID parent)
{
	RE_Component* ret = nullptr;

	switch (copy->GetType())
	{
	case RE_Component::Type::TRANSFORM: ret = static_cast<RE_Component*>(transPool.AtPtr(transPool.GetNewCompUID())); break;
	case RE_Component::Type::CAMERA: ret = static_cast<RE_Component*>(camPool.AtPtr(camPool.GetNewCompUID())); break;
	case RE_Component::Type::MESH: ret = static_cast<RE_Component*>(meshPool.AtPtr(meshPool.GetNewCompUID())); break;
	case RE_Component::Type::LIGHT: ret = static_cast<RE_Component*>(lightPool.AtPtr(lightPool.GetNewCompUID())); break;
	case RE_Component::Type::WATER: ret = static_cast<RE_Component*>(waterPool.AtPtr(waterPool.GetNewCompUID())); break;
	case RE_Component::Type::PARTICLEEMITER: ret = static_cast<RE_Component*>(particleSPool.AtPtr(particleSPool.GetNewCompUID())); break;
	case RE_Component::Type::GRID: ret = static_cast<RE_Component*>(pGridPool.AtPtr(pGridPool.GetNewCompUID())); break;
	case RE_Component::Type::ROCK: ret = static_cast<RE_Component*>(pRockPool.AtPtr(pRockPool.GetNewCompUID())); break;
	case RE_Component::Type::CUBE: ret = static_cast<RE_Component*>(pCubePool.AtPtr(pCubePool.GetNewCompUID())); break;
	case RE_Component::Type::DODECAHEDRON: ret = static_cast<RE_Component*>(pDodecahedronPool.AtPtr(pDodecahedronPool.GetNewCompUID())); break;
	case RE_Component::Type::TETRAHEDRON: ret = static_cast<RE_Component*>(pTetrahedronPool.AtPtr(pTetrahedronPool.GetNewCompUID())); break;
	case RE_Component::Type::OCTOHEDRON: ret = static_cast<RE_Component*>(pOctohedronPool.AtPtr(pOctohedronPool.GetNewCompUID())); break;
	case RE_Component::Type::ICOSAHEDRON: ret = static_cast<RE_Component*>(pIcosahedronPool.AtPtr(pIcosahedronPool.GetNewCompUID())); break;
	case RE_Component::Type::POINT: ret = static_cast<RE_Component*>(pPointPool.AtPtr(pPointPool.GetNewCompUID())); break;
	case RE_Component::Type::PLANE: ret = static_cast<RE_Component*>(pPlanePool.AtPtr(pPlanePool.GetNewCompUID())); break;
	case RE_Component::Type::SPHERE: ret = static_cast<RE_Component*>(pSpherePool.AtPtr(pSpherePool.GetNewCompUID())); break;
	case RE_Component::Type::CYLINDER: ret = static_cast<RE_Component*>(pCylinderPool.AtPtr(pCylinderPool.GetNewCompUID())); break;
	case RE_Component::Type::HEMISHPERE: ret = static_cast<RE_Component*>(pHemiSpherePool.AtPtr(pHemiSpherePool.GetNewCompUID())); break;
	case RE_Component::Type::TORUS: ret = static_cast<RE_Component*>(pTorusPool.AtPtr(pTorusPool.GetNewCompUID())); break;
	case RE_Component::Type::TREFOILKNOT: ret = static_cast<RE_Component*>(pTrefoiKnotPool.AtPtr(pTrefoiKnotPool.GetNewCompUID())); break;
	default: return nullptr;
	}

	ret->CopySetUp(pool, copy, parent);
	return ret;
}

void ComponentsPool::DestroyComponent(RE_Component::Type cType, COMP_UID toDelete)
{
	RE_Component* cmp = GetComponentPtr(toDelete, cType);
	cmp->GetGOPtr()->ReleaseComponent(toDelete, cType);

	switch (cType)
	{
	case RE_Component::Type::TRANSFORM: transPool.Pop(toDelete); break;
	case RE_Component::Type::CAMERA: camPool.Pop(toDelete); break;
	case RE_Component::Type::MESH: meshPool.Pop(toDelete); break;
	case RE_Component::Type::LIGHT: lightPool.Pop(toDelete); break;
	case RE_Component::Type::WATER: waterPool.Pop(toDelete); break;
	case RE_Component::Type::PARTICLEEMITER: particleSPool.Pop(toDelete); break;
	case RE_Component::Type::GRID: pGridPool.Pop(toDelete); break;
	case RE_Component::Type::ROCK: pRockPool.Pop(toDelete); break;
	case RE_Component::Type::CUBE: pCubePool.Pop(toDelete); break;
	case RE_Component::Type::DODECAHEDRON: pDodecahedronPool.Pop(toDelete); break;
	case RE_Component::Type::TETRAHEDRON: pTetrahedronPool.Pop(toDelete); break;
	case RE_Component::Type::OCTOHEDRON: pOctohedronPool.Pop(toDelete); break;
	case RE_Component::Type::ICOSAHEDRON: pIcosahedronPool.Pop(toDelete); break;
	case RE_Component::Type::POINT:  pPointPool.Pop(toDelete); break;
	case RE_Component::Type::PLANE:  pPlanePool.Pop(toDelete); break;
	case RE_Component::Type::SPHERE: pSpherePool.Pop(toDelete); break;
	case RE_Component::Type::CYLINDER: pCylinderPool.Pop(toDelete); break;
	case RE_Component::Type::HEMISHPERE: pHemiSpherePool.Pop(toDelete); break;
	case RE_Component::Type::TORUS: pTorusPool.Pop(toDelete); break;
	case RE_Component::Type::TREFOILKNOT: pTrefoiKnotPool.Pop(toDelete); break;
	default: break;
	}
}

eastl::vector<COMP_UID> ComponentsPool::GetAllCompUID(RE_Component::Type type) const
{
	eastl::vector<COMP_UID> ret;

	switch (type)
	{
	case RE_Component::Type::TRANSFORM: ret = transPool.GetAllKeys(); break;
	case RE_Component::Type::CAMERA: ret = camPool.GetAllKeys(); break;
	case RE_Component::Type::MESH: ret = meshPool.GetAllKeys(); break;
	case RE_Component::Type::LIGHT: ret = lightPool.GetAllKeys(); break;
	case RE_Component::Type::WATER: ret = waterPool.GetAllKeys(); break;
	case RE_Component::Type::PARTICLEEMITER: ret = particleSPool.GetAllKeys(); break;
	case RE_Component::Type::FUSTRUM: break;
	case RE_Component::Type::GRID: ret = pGridPool.GetAllKeys(); break;
	case RE_Component::Type::ROCK: ret = pRockPool.GetAllKeys(); break;
	case RE_Component::Type::CUBE: ret = pCubePool.GetAllKeys(); break;
	case RE_Component::Type::DODECAHEDRON: ret = pDodecahedronPool.GetAllKeys(); break;
	case RE_Component::Type::TETRAHEDRON: ret = pTetrahedronPool.GetAllKeys(); break;
	case RE_Component::Type::OCTOHEDRON: ret = pOctohedronPool.GetAllKeys(); break;
	case RE_Component::Type::ICOSAHEDRON: ret = pIcosahedronPool.GetAllKeys(); break;
	case RE_Component::Type::POINT: ret = pPointPool.GetAllKeys(); break;
	case RE_Component::Type::PLANE: ret = pPlanePool.GetAllKeys(); break;
	case RE_Component::Type::SPHERE: ret = pSpherePool.GetAllKeys(); break;
	case RE_Component::Type::CYLINDER: ret = pCylinderPool.GetAllKeys(); break;
	case RE_Component::Type::HEMISHPERE: ret = pHemiSpherePool.GetAllKeys(); break;
	case RE_Component::Type::TORUS: ret = pTorusPool.GetAllKeys(); break;
	case RE_Component::Type::TREFOILKNOT: ret = pTrefoiKnotPool.GetAllKeys(); break;
	default: break;
	}

	return ret;
}

eastl::vector<RE_Component*> ComponentsPool::GetAllCompPtr(RE_Component::Type type) const
{
	eastl::vector<RE_Component*> ret;

	switch (type)
	{
	case RE_Component::Type::TRANSFORM:
		for (auto id : transPool.GetAllKeys())
			ret.push_back(static_cast<RE_Component*>(transPool.AtPtr(id)));
		break;
	case RE_Component::Type::CAMERA:
		for (auto id : camPool.GetAllKeys())
			ret.push_back(static_cast<RE_Component*>(camPool.AtPtr(id)));
		break;
	case RE_Component::Type::MESH:
		for (auto id : meshPool.GetAllKeys())
			ret.push_back(static_cast<RE_Component*>(meshPool.AtPtr(id)));
		break;
	case RE_Component::Type::LIGHT:
		for (auto id : lightPool.GetAllKeys())
			ret.push_back(static_cast<RE_Component*>(lightPool.AtPtr(id)));
		break;
	case RE_Component::Type::WATER:
		for (auto id : waterPool.GetAllKeys())
			ret.push_back(static_cast<RE_Component*>(waterPool.AtPtr(id)));
		break;
	case RE_Component::Type::PARTICLEEMITER:
		for (auto id : particleSPool.GetAllKeys())
			ret.push_back(static_cast<RE_Component*>(particleSPool.AtPtr(id)));
		break;
	case RE_Component::Type::GRID:
		for (auto id : pGridPool.GetAllKeys())
			ret.push_back(static_cast<RE_Component*>(pGridPool.AtPtr(id)));
		break;
	case RE_Component::Type::ROCK:
		for (auto id : pRockPool.GetAllKeys())
			ret.push_back(static_cast<RE_Component*>(pRockPool.AtPtr(id)));
		break;
	case RE_Component::Type::CUBE:
		for (auto id : pCubePool.GetAllKeys())
			ret.push_back(static_cast<RE_Component*>(pCubePool.AtPtr(id)));
		break;
	case RE_Component::Type::DODECAHEDRON:
		for (auto id : pDodecahedronPool.GetAllKeys())
			ret.push_back(static_cast<RE_Component*>(pDodecahedronPool.AtPtr(id)));
		break;
	case RE_Component::Type::TETRAHEDRON:
		for (auto id : pTetrahedronPool.GetAllKeys())
			ret.push_back(static_cast<RE_Component*>(pTetrahedronPool.AtPtr(id)));
		break;
	case RE_Component::Type::OCTOHEDRON:
		for (auto id : pOctohedronPool.GetAllKeys())
			ret.push_back(static_cast<RE_Component*>(pOctohedronPool.AtPtr(id)));
		break;
	case RE_Component::Type::ICOSAHEDRON:
		for (auto id : pIcosahedronPool.GetAllKeys())
			ret.push_back(static_cast<RE_Component*>(pIcosahedronPool.AtPtr(id)));
		break;
	case RE_Component::Type::POINT:
		for (auto id : pPointPool.GetAllKeys())
			ret.push_back(static_cast<RE_Component*>(pPointPool.AtPtr(id)));
		break;
	case RE_Component::Type::PLANE:
		for (auto id : pPlanePool.GetAllKeys())
			ret.push_back(static_cast<RE_Component*>(pPlanePool.AtPtr(id)));
		break;
	case RE_Component::Type::SPHERE:
		for (auto id : pSpherePool.GetAllKeys())
			ret.push_back(static_cast<RE_Component*>(pSpherePool.AtPtr(id)));
		break;
	case RE_Component::Type::CYLINDER:
		for (auto id : pCylinderPool.GetAllKeys())
			ret.push_back(static_cast<RE_Component*>(pCylinderPool.AtPtr(id)));
		break;
	case RE_Component::Type::HEMISHPERE:
		for (auto id : pHemiSpherePool.GetAllKeys())
			ret.push_back(static_cast<RE_Component*>(pHemiSpherePool.AtPtr(id)));
		break;
	case RE_Component::Type::TORUS:
		for (auto id : pTorusPool.GetAllKeys())
			ret.push_back(static_cast<RE_Component*>(pTorusPool.AtPtr(id)));
		break;
	case RE_Component::Type::TREFOILKNOT:
		for (auto id : pTrefoiKnotPool.GetAllKeys())
		ret.push_back(static_cast<RE_Component*>(pTrefoiKnotPool.AtPtr(id)));
		break;
	default: break;
	}

	return ret;
}

eastl::vector<const RE_Component*> ComponentsPool::GetAllCompCPtr(RE_Component::Type type) const
{
	eastl::vector<const RE_Component*> ret;

	switch (type)
	{
	case RE_Component::Type::TRANSFORM:
		for (auto id : transPool.GetAllKeys())
			ret.push_back(static_cast<const RE_Component*>(transPool.AtPtr(id)));
		break;
	case RE_Component::Type::CAMERA:
		for (auto id : camPool.GetAllKeys())
			ret.push_back(static_cast<const RE_Component*>(camPool.AtPtr(id)));
		break;
	case RE_Component::Type::MESH:
		for (auto id : meshPool.GetAllKeys())
			ret.push_back(static_cast<const RE_Component*>(meshPool.AtPtr(id)));
		break;
	case RE_Component::Type::LIGHT:
		for (auto id : lightPool.GetAllKeys())
			ret.push_back(static_cast<const RE_Component*>(lightPool.AtPtr(id)));
		break;
	case RE_Component::Type::WATER:
		for (auto id : waterPool.GetAllKeys())
			ret.push_back(static_cast<const RE_Component*>(waterPool.AtPtr(id)));
		break;
	case RE_Component::Type::PARTICLEEMITER:
		for (auto id : particleSPool.GetAllKeys())
			ret.push_back(static_cast<const RE_Component*>(particleSPool.AtPtr(id)));
		break;
	case RE_Component::Type::GRID:
		for (auto id : pGridPool.GetAllKeys())
			ret.push_back(static_cast<const RE_Component*>(pGridPool.AtPtr(id)));
		break;
	case RE_Component::Type::ROCK:
		for (auto id : pRockPool.GetAllKeys())
			ret.push_back(static_cast<const RE_Component*>(pRockPool.AtPtr(id)));
		break;
	case RE_Component::Type::CUBE:
		for (auto id : pCubePool.GetAllKeys())
			ret.push_back(static_cast<const RE_Component*>(pCubePool.AtPtr(id)));
		break;
	case RE_Component::Type::DODECAHEDRON:
		for (auto id : pDodecahedronPool.GetAllKeys())
			ret.push_back(static_cast<const RE_Component*>(pDodecahedronPool.AtPtr(id)));
		break;
	case RE_Component::Type::TETRAHEDRON:
		for (auto id : pTetrahedronPool.GetAllKeys())
			ret.push_back(static_cast<const RE_Component*>(pTetrahedronPool.AtPtr(id)));
		break;
	case RE_Component::Type::OCTOHEDRON:
		for (auto id : pOctohedronPool.GetAllKeys())
			ret.push_back(static_cast<const RE_Component*>(pOctohedronPool.AtPtr(id)));
		break;
	case RE_Component::Type::ICOSAHEDRON:
		for (auto id : pIcosahedronPool.GetAllKeys())
			ret.push_back(static_cast<const RE_Component*>(pIcosahedronPool.AtPtr(id)));
		break;
	case RE_Component::Type::POINT:
		for (auto id : pPointPool.GetAllKeys())
			ret.push_back(static_cast<const RE_Component*>(pPointPool.AtPtr(id)));
		break;
	case RE_Component::Type::PLANE:
		for (auto id : pPlanePool.GetAllKeys())
			ret.push_back(static_cast<const RE_Component*>(pPlanePool.AtPtr(id)));
		break;
	case RE_Component::Type::SPHERE:
		for (auto id : pSpherePool.GetAllKeys())
			ret.push_back(static_cast<const RE_Component*>(pSpherePool.AtPtr(id)));
		break;
	case RE_Component::Type::CYLINDER:
		for (auto id : pCylinderPool.GetAllKeys())
			ret.push_back(static_cast<const RE_Component*>(pCylinderPool.AtPtr(id)));
		break;
	case RE_Component::Type::HEMISHPERE:
		for (auto id : pHemiSpherePool.GetAllKeys())
			ret.push_back(static_cast<const RE_Component*>(pHemiSpherePool.AtPtr(id)));
		break;
	case RE_Component::Type::TORUS:
		for (auto id : pTorusPool.GetAllKeys())
			ret.push_back(static_cast<const RE_Component*>(pTorusPool.AtPtr(id)));
		break;
	case RE_Component::Type::TREFOILKNOT:
		for (auto id : pTrefoiKnotPool.GetAllKeys())
			ret.push_back(static_cast<const RE_Component*>(pTrefoiKnotPool.AtPtr(id)));
		break;
	default: break;
	}

	return ret;
}

eastl::vector<eastl::pair<const COMP_UID, RE_Component*>> ComponentsPool::GetAllCompData(RE_Component::Type type) const
{
	eastl::vector<eastl::pair<const COMP_UID, RE_Component*>> ret;

	switch (type)
	{
	case RE_Component::Type::TRANSFORM:
		for (auto id : transPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(transPool.AtPtr(id)) });
		break;
	case RE_Component::Type::CAMERA:
		for (auto id : camPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(camPool.AtPtr(id)) });
		break;
	case RE_Component::Type::MESH:
		for (auto id : meshPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(meshPool.AtPtr(id)) });
		break;
	case RE_Component::Type::LIGHT:
		for (auto id : lightPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(lightPool.AtPtr(id)) });
		break;
	case RE_Component::Type::WATER:
		for (auto id : waterPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(waterPool.AtPtr(id)) });
		break;
	case RE_Component::Type::PARTICLEEMITER:
		for (auto id : particleSPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(particleSPool.AtPtr(id)) });
		break;
	case RE_Component::Type::GRID:
		for (auto id : pGridPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pGridPool.AtPtr(id)) });
		break;
	case RE_Component::Type::ROCK:
		for (auto id : pRockPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pRockPool.AtPtr(id)) });
		break;
	case RE_Component::Type::CUBE:
		for (auto id : pCubePool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pCubePool.AtPtr(id)) });
		break;
	case RE_Component::Type::DODECAHEDRON:
		for (auto id : pDodecahedronPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pDodecahedronPool.AtPtr(id)) });
		break;
	case RE_Component::Type::TETRAHEDRON:
		for (auto id : pTetrahedronPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pTetrahedronPool.AtPtr(id)) });
		break;
	case RE_Component::Type::OCTOHEDRON:
		for (auto id : pOctohedronPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pOctohedronPool.AtPtr(id)) });
		break;
	case RE_Component::Type::ICOSAHEDRON:
		for (auto id : pIcosahedronPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pIcosahedronPool.AtPtr(id)) });
		break;
	case RE_Component::Type::POINT:
		for (auto id : pPointPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pPointPool.AtPtr(id)) });
		break;
	case RE_Component::Type::PLANE:
		for (auto id : pPlanePool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pPlanePool.AtPtr(id)) });
		break;
	case RE_Component::Type::SPHERE:
		for (auto id : pSpherePool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pSpherePool.AtPtr(id)) });
		break;
	case RE_Component::Type::CYLINDER:
		for (auto id : pCylinderPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pCylinderPool.AtPtr(id)) });
		break;
	case RE_Component::Type::HEMISHPERE:
		for (auto id : pHemiSpherePool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pHemiSpherePool.AtPtr(id)) });
		break;
	case RE_Component::Type::TORUS:
		for (auto id : pTorusPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pTorusPool.AtPtr(id)) });
		break;
	case RE_Component::Type::TREFOILKNOT:
		for (auto id : pTrefoiKnotPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pTrefoiKnotPool.AtPtr(id)) });
		break;
	default: break;
	}

	return ret;
}

eastl::vector<eastl::pair<const COMP_UID, const RE_Component*>> ComponentsPool::GetAllCompCData(RE_Component::Type type) const
{
	eastl::vector<eastl::pair<const COMP_UID, const RE_Component*>> ret;

	switch (type)
	{
	case RE_Component::Type::TRANSFORM:
		for (auto id : transPool.GetAllKeys())
			ret.push_back({ id, static_cast<const RE_Component*>(transPool.AtCPtr(id)) });
		break;
	case RE_Component::Type::CAMERA:
		for (auto id : camPool.GetAllKeys())
			ret.push_back({ id, static_cast<const RE_Component*>(camPool.AtCPtr(id)) });
		break;
	case RE_Component::Type::MESH:
		for (auto id : meshPool.GetAllKeys())
			ret.push_back({ id, static_cast<const RE_Component*>(meshPool.AtCPtr(id)) });
		break;
	case RE_Component::Type::LIGHT:
		for (auto id : lightPool.GetAllKeys())
			ret.push_back({ id, static_cast<const RE_Component*>(lightPool.AtCPtr(id)) });
		break;
	case RE_Component::Type::WATER:
		for (auto id : waterPool.GetAllKeys())
			ret.push_back({ id, static_cast<const RE_Component*>(waterPool.AtCPtr(id)) });
		break;
	case RE_Component::Type::PARTICLEEMITER:
		for (auto id : particleSPool.GetAllKeys())
			ret.push_back({ id, static_cast<const RE_Component*>(particleSPool.AtCPtr(id)) });
		break;
	case RE_Component::Type::GRID:
		for (auto id : pGridPool.GetAllKeys())
			ret.push_back({ id, static_cast<const RE_Component*>(pGridPool.AtCPtr(id)) });
		break;
	case RE_Component::Type::ROCK:
		for (auto id : pRockPool.GetAllKeys())
			ret.push_back({ id, static_cast<const RE_Component*>(pRockPool.AtCPtr(id)) });
		break;
	case RE_Component::Type::CUBE:
		for (auto id : pCubePool.GetAllKeys())
			ret.push_back({ id, static_cast<const RE_Component*>(pCubePool.AtCPtr(id)) });
		break;
	case RE_Component::Type::DODECAHEDRON:
		for (auto id : pDodecahedronPool.GetAllKeys())
			ret.push_back({ id, static_cast<const RE_Component*>(pDodecahedronPool.AtCPtr(id)) });
		break;
	case RE_Component::Type::TETRAHEDRON:
		for (auto id : pTetrahedronPool.GetAllKeys())
			ret.push_back({ id, static_cast<const RE_Component*>(pTetrahedronPool.AtCPtr(id)) });
		break;
	case RE_Component::Type::OCTOHEDRON:
		for (auto id : pOctohedronPool.GetAllKeys())
			ret.push_back({ id, static_cast<const RE_Component*>(pOctohedronPool.AtCPtr(id)) });
		break;
	case RE_Component::Type::ICOSAHEDRON:
		for (auto id : pIcosahedronPool.GetAllKeys())
			ret.push_back({ id, static_cast<const RE_Component*>(pIcosahedronPool.AtCPtr(id)) });
		break;
	case RE_Component::Type::POINT:
		for (auto id : pPointPool.GetAllKeys())
			ret.push_back({ id, static_cast<const RE_Component*>(pPointPool.AtCPtr(id)) });
		break;
	case RE_Component::Type::PLANE:
		for (auto id : pPlanePool.GetAllKeys())
			ret.push_back({ id, static_cast<const RE_Component*>(pPlanePool.AtCPtr(id)) });
		break;
	case RE_Component::Type::SPHERE:
		for (auto id : pSpherePool.GetAllKeys())
			ret.push_back({ id, static_cast<const RE_Component*>(pSpherePool.AtCPtr(id)) });
		break;
	case RE_Component::Type::CYLINDER:
		for (auto id : pCylinderPool.GetAllKeys())
			ret.push_back({ id, static_cast<const RE_Component*>(pCylinderPool.AtCPtr(id)) });
		break;
	case RE_Component::Type::HEMISHPERE:
		for (auto id : pHemiSpherePool.GetAllKeys())
			ret.push_back({ id, static_cast<const RE_Component*>(pHemiSpherePool.AtCPtr(id)) });
		break;
	case RE_Component::Type::TORUS:
		for (auto id : pTorusPool.GetAllKeys())
			ret.push_back({ id, static_cast<const RE_Component*>(pTorusPool.AtCPtr(id)) });
		break;
	case RE_Component::Type::TREFOILKNOT:
		for (auto id : pTrefoiKnotPool.GetAllKeys())
			ret.push_back({ id, static_cast<const RE_Component*>(pTrefoiKnotPool.AtCPtr(id)) });
		break;
	default: break;
	}

	return ret;
}

void ComponentsPool::SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources)
{
	RE_Json* comps = node->PushJObject("Components Pool");
	camPool.SerializeJson(comps, resources);
	meshPool.SerializeJson(comps, resources);
	lightPool.SerializeJson(comps, resources);
	waterPool.SerializeJson(comps, resources);
	particleSPool.SerializeJson(comps, resources);
	pGridPool.SerializeJson(comps, resources);
	pRockPool.SerializeJson(comps, resources);
	pCubePool.SerializeJson(comps, resources);
	pDodecahedronPool.SerializeJson(comps, resources);
	pTetrahedronPool.SerializeJson(comps, resources);
	pOctohedronPool.SerializeJson(comps, resources);
	pIcosahedronPool.SerializeJson(comps, resources);
	pPointPool.SerializeJson(comps, resources);
	pPlanePool.SerializeJson(comps, resources);
	pSpherePool.SerializeJson(comps, resources);
	pCylinderPool.SerializeJson(comps, resources);
	pHemiSpherePool.SerializeJson(comps, resources);
	pTorusPool.SerializeJson(comps, resources);
	pTrefoiKnotPool.SerializeJson(comps, resources);
	DEL(comps)
}

void ComponentsPool::DeserializeJson(GameObjectsPool* goPool, RE_Json* node, eastl::map<int, const char*>* resources)
{
	RE_Json* comps = node->PullJObject("Components Pool");
	camPool.DeserializeJson(goPool, comps, resources);
	meshPool.DeserializeJson(goPool, comps, resources);
	lightPool.DeserializeJson(goPool, comps, resources);
	waterPool.DeserializeJson(goPool, comps, resources);
	particleSPool.DeserializeJson(goPool, comps, resources);
	pGridPool.DeserializeJson(goPool, comps, resources);
	pRockPool.DeserializeJson(goPool, comps, resources);
	pCubePool.DeserializeJson(goPool, comps, resources);
	pDodecahedronPool.DeserializeJson(goPool, comps, resources);
	pTetrahedronPool.DeserializeJson(goPool, comps, resources);
	pOctohedronPool.DeserializeJson(goPool, comps, resources);
	pIcosahedronPool.DeserializeJson(goPool, comps, resources);
	pPointPool.DeserializeJson(goPool, comps, resources);
	pPlanePool.DeserializeJson(goPool, comps, resources);
	pSpherePool.DeserializeJson(goPool, comps, resources);
	pCylinderPool.DeserializeJson(goPool, comps, resources);
	pHemiSpherePool.DeserializeJson(goPool, comps, resources);
	pTorusPool.DeserializeJson(goPool, comps, resources);
	pTrefoiKnotPool.DeserializeJson(goPool, comps, resources);
	DEL(comps)
}

size_t ComponentsPool::GetBinarySize() const
{
	size_t size = 0;
	size += camPool.GetBinarySize();
	size += meshPool.GetBinarySize();
	size += lightPool.GetBinarySize();
	size += waterPool.GetBinarySize();
	size += particleSPool.GetBinarySize();
	size += pGridPool.GetBinarySize();
	size += pRockPool.GetBinarySize();
	size += pCubePool.GetBinarySize();
	size += pDodecahedronPool.GetBinarySize();
	size += pTetrahedronPool.GetBinarySize();
	size += pOctohedronPool.GetBinarySize();
	size += pIcosahedronPool.GetBinarySize();
	size += pPointPool.GetBinarySize();
	size += pPlanePool.GetBinarySize();
	size += pSpherePool.GetBinarySize();
	size += pCylinderPool.GetBinarySize();
	size += pHemiSpherePool.GetBinarySize();
	size += pTorusPool.GetBinarySize();
	size += pTrefoiKnotPool.GetBinarySize();
	return size;
}

void ComponentsPool::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources)
{
	camPool.SerializeBinary(cursor, resources);
	meshPool.SerializeBinary(cursor, resources);
	lightPool.SerializeBinary(cursor, resources);
	waterPool.SerializeBinary(cursor, resources);
	particleSPool.SerializeBinary(cursor, resources);
	pGridPool.SerializeBinary(cursor, resources);
	pRockPool.SerializeBinary(cursor, resources);
	pCubePool.SerializeBinary(cursor, resources);
	pDodecahedronPool.SerializeBinary(cursor, resources);
	pTetrahedronPool.SerializeBinary(cursor, resources);
	pOctohedronPool.SerializeBinary(cursor, resources);
	pIcosahedronPool.SerializeBinary(cursor, resources);
	pPointPool.SerializeBinary(cursor, resources);
	pPlanePool.SerializeBinary(cursor, resources);
	pSpherePool.SerializeBinary(cursor, resources);
	pCylinderPool.SerializeBinary(cursor, resources);
	pHemiSpherePool.SerializeBinary(cursor, resources);
	pTorusPool.SerializeBinary(cursor, resources);
	pTrefoiKnotPool.SerializeBinary(cursor, resources);
}

void ComponentsPool::DeserializeBinary(GameObjectsPool* goPool, char*& cursor, eastl::map<int, const char*>* resources)
{
	camPool.DeserializeBinary(goPool, cursor, resources);
	meshPool.DeserializeBinary(goPool, cursor, resources);
	lightPool.DeserializeBinary(goPool, cursor, resources);
	waterPool.DeserializeBinary(goPool, cursor, resources);
	particleSPool.DeserializeBinary(goPool, cursor, resources);
	pGridPool.DeserializeBinary(goPool, cursor, resources);
	pRockPool.DeserializeBinary(goPool, cursor, resources);
	pCubePool.DeserializeBinary(goPool, cursor, resources);
	pDodecahedronPool.DeserializeBinary(goPool, cursor, resources);
	pTetrahedronPool.DeserializeBinary(goPool, cursor, resources);
	pOctohedronPool.DeserializeBinary(goPool, cursor, resources);
	pIcosahedronPool.DeserializeBinary(goPool, cursor, resources);
	pPointPool.DeserializeBinary(goPool, cursor, resources);
	pPlanePool.DeserializeBinary(goPool, cursor, resources);
	pSpherePool.DeserializeBinary(goPool, cursor, resources);
	pCylinderPool.DeserializeBinary(goPool, cursor, resources);
	pHemiSpherePool.DeserializeBinary(goPool, cursor, resources);
	pTorusPool.DeserializeBinary(goPool, cursor, resources);
	pTrefoiKnotPool.DeserializeBinary(goPool, cursor, resources);
}