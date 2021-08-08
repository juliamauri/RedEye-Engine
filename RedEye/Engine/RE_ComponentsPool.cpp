#include "RE_ComponentsPool.h"

#include "RE_Memory.h"
#include "RE_Json.h"
#include "RE_GameObject.h"

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
	eastl::vector<const char*> resources = camPool.GetAllResources();
	if (!resources.empty()) allResources.insert(allResources.end(), resources.begin(), resources.end());
	resources = meshPool.GetAllResources();
	if (!resources.empty()) allResources.insert(allResources.end(), resources.begin(), resources.end());
	resources = particleSPool.GetAllResources();
	if (!resources.empty()) allResources.insert(allResources.end(), resources.begin(), resources.end());

	eastl::vector<const char*> ret;
	int resSize = 0;
	for (auto res : allResources) {
		bool repeat = false;
		for (auto uniqueRes : ret) {
			resSize = eastl::CharStrlen(res);
			if (resSize > 0 && eastl::Compare(res, uniqueRes, resSize) == 0) {
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

RE_Component* ComponentsPool::GetComponentPtr(COMP_UID poolid, ComponentType cType)
{
	RE_Component* ret = nullptr;
	switch (cType) {
	case C_TRANSFORM: ret = static_cast<RE_Component*>(transPool.AtPtr(poolid)); break;
	case C_CAMERA: ret = static_cast<RE_Component*>(camPool.AtPtr(poolid)); break;
	case C_MESH: ret = static_cast<RE_Component*>(meshPool.AtPtr(poolid)); break;
	case C_LIGHT: ret = static_cast<RE_Component*>(lightPool.AtPtr(poolid)); break;
	case C_WATER: ret = static_cast<RE_Component*>(waterPool.AtPtr(poolid)); break;
	case C_PARTICLEEMITER: ret = static_cast<RE_Component*>(particleSPool.AtPtr(poolid)); break;
	case C_FUSTRUM: break;
	case C_GRID: ret = static_cast<RE_Component*>(pGridPool.AtPtr(poolid)); break;
	case C_ROCK: ret = static_cast<RE_Component*>(pRockPool.AtPtr(poolid)); break;
	case C_CUBE: ret = static_cast<RE_Component*>(pCubePool.AtPtr(poolid)); break;
	case C_DODECAHEDRON: ret = static_cast<RE_Component*>(pDodecahedronPool.AtPtr(poolid)); break;
	case C_TETRAHEDRON: ret = static_cast<RE_Component*>(pTetrahedronPool.AtPtr(poolid)); break;
	case C_OCTOHEDRON: ret = static_cast<RE_Component*>(pOctohedronPool.AtPtr(poolid)); break;
	case C_ICOSAHEDRON: ret = static_cast<RE_Component*>(pIcosahedronPool.AtPtr(poolid)); break;
	case C_POINT: ret = static_cast<RE_Component*>(pPointPool.AtPtr(poolid)); break;
	case C_PLANE: ret = static_cast<RE_Component*>(pPlanePool.AtPtr(poolid)); break;
	case C_SPHERE: ret = static_cast<RE_Component*>(pSpherePool.AtPtr(poolid)); break;
	case C_CYLINDER: ret = static_cast<RE_Component*>(pCylinderPool.AtPtr(poolid)); break;
	case C_HEMISHPERE: ret = static_cast<RE_Component*>(pHemiSpherePool.AtPtr(poolid)); break;
	case C_TORUS: ret = static_cast<RE_Component*>(pTorusPool.AtPtr(poolid)); break;
	case C_TREFOILKNOT:  ret = static_cast<RE_Component*>(pTrefoiKnotPool.AtPtr(poolid)); break;
	}
	return ret;
}

const RE_Component* ComponentsPool::GetComponentCPtr(COMP_UID poolid, ComponentType cType) const
{
	const RE_Component* ret = nullptr;
	switch (cType) {
	case C_TRANSFORM: ret = static_cast<const RE_Component*>(transPool.AtCPtr(poolid)); break;
	case C_CAMERA: ret = static_cast<const RE_Component*>(camPool.AtCPtr(poolid)); break;
	case C_MESH: ret = static_cast<const RE_Component*>(meshPool.AtCPtr(poolid)); break;
	case C_LIGHT: ret = static_cast<const RE_Component*>(lightPool.AtCPtr(poolid)); break;
	case C_WATER: ret = static_cast<const RE_Component*>(waterPool.AtCPtr(poolid)); break;
	case C_PARTICLEEMITER: ret = static_cast<const RE_Component*>(particleSPool.AtCPtr(poolid)); break;
	case C_FUSTRUM:  break;
	case C_GRID: ret = static_cast<const RE_Component*>(pGridPool.AtCPtr(poolid)); break;
	case C_ROCK: ret = static_cast<const RE_Component*>(pRockPool.AtCPtr(poolid)); break;
	case C_CUBE: ret = static_cast<const RE_Component*>(pCubePool.AtCPtr(poolid)); break;
	case C_DODECAHEDRON: ret = static_cast<const RE_Component*>(pDodecahedronPool.AtCPtr(poolid)); break;
	case C_TETRAHEDRON: ret = static_cast<const RE_Component*>(pTetrahedronPool.AtCPtr(poolid)); break;
	case C_OCTOHEDRON: ret = static_cast<const RE_Component*>(pOctohedronPool.AtCPtr(poolid)); break;
	case C_ICOSAHEDRON: ret = static_cast<const RE_Component*>(pIcosahedronPool.AtCPtr(poolid)); break;
	case C_POINT: ret = static_cast<const RE_Component*>(pPointPool.AtCPtr(poolid)); break;
	case C_PLANE: ret = static_cast<const RE_Component*>(pPlanePool.AtCPtr(poolid)); break;
	case C_SPHERE: ret = static_cast<const RE_Component*>(pSpherePool.AtCPtr(poolid)); break;
	case C_CYLINDER: ret = static_cast<const RE_Component*>(pCylinderPool.AtCPtr(poolid)); break;
	case C_HEMISHPERE: ret = static_cast<const RE_Component*>(pHemiSpherePool.AtCPtr(poolid)); break;
	case C_TORUS: ret = static_cast<const RE_Component*>(pTorusPool.AtCPtr(poolid)); break;
	case C_TREFOILKNOT: ret = static_cast<const RE_Component*>(pTrefoiKnotPool.AtCPtr(poolid)); break;
	}
	return ret;
}

eastl::pair<const COMP_UID, RE_Component*> ComponentsPool::GetNewComponent(ComponentType cType)
{
	switch (cType)
	{
	case C_TRANSFORM:
	{
		const COMP_UID id = transPool.GetNewCompUID();
		return { id, transPool.AtPtr(id) };
	}
	case C_CAMERA:
	{
		const COMP_UID id = camPool.GetNewCompUID();
		return { id, camPool.AtPtr(id) };
	}
	case C_MESH:
	{
		const COMP_UID id = meshPool.GetNewCompUID();
		return { id, meshPool.AtPtr(id) };
	}
	case C_LIGHT:
	{
		const COMP_UID id = lightPool.GetNewCompUID();
		return { id, lightPool.AtPtr(id) };
	}
	case C_WATER:
	{
		const COMP_UID id = waterPool.GetNewCompUID();
		return { id, waterPool.AtPtr(id) };
	}
	case C_PARTICLEEMITER:
	{
		const COMP_UID id = particleSPool.GetNewCompUID();
		return { id, particleSPool.AtPtr(id) };
	}
	case C_GRID:
	{
		const COMP_UID id = pGridPool.GetNewCompUID();
		return { id, pGridPool.AtPtr(id) };
	}
	case C_ROCK:
	{
		const COMP_UID id = pRockPool.GetNewCompUID();
		return { id, pRockPool.AtPtr(id) };
	}
	case C_CUBE:
	{
		const COMP_UID id = pCubePool.GetNewCompUID();
		return { id, pCubePool.AtPtr(id) };
	}
	case C_DODECAHEDRON:
	{
		const COMP_UID id = pDodecahedronPool.GetNewCompUID();
		return { id, pDodecahedronPool.AtPtr(id) };
	}
	case C_TETRAHEDRON:
	{
		const COMP_UID id = pTetrahedronPool.GetNewCompUID();
		return { id, pTetrahedronPool.AtPtr(id) };
	}
	case C_OCTOHEDRON:
	{
		const COMP_UID id = pOctohedronPool.GetNewCompUID();
		return { id, pOctohedronPool.AtPtr(id) };
	}
	case C_ICOSAHEDRON:
	{
		const COMP_UID id = pIcosahedronPool.GetNewCompUID();
		return { id, pIcosahedronPool.AtPtr(id) };
	}
	case C_POINT:
	{
		const COMP_UID id = pPointPool.GetNewCompUID();
		return { id, pPointPool.AtPtr(id) };
	}
	case C_PLANE:
	{
		const COMP_UID id = pPlanePool.GetNewCompUID();
		return { id, pPlanePool.AtPtr(id) };
	}
	case C_SPHERE:
	{
		const COMP_UID id = pSpherePool.GetNewCompUID();
		return { id, pSpherePool.AtPtr(id) };
	}
	case C_CYLINDER:
	{
		const COMP_UID id = pCylinderPool.GetNewCompUID();
		return { id, pCylinderPool.AtPtr(id) };
	}
	case C_HEMISHPERE:
	{
		const COMP_UID id = pHemiSpherePool.GetNewCompUID();
		return { id, pHemiSpherePool.AtPtr(id) };
	}
	case C_TORUS:
	{
		const COMP_UID id = pTorusPool.GetNewCompUID();
		return { id, pTorusPool.AtPtr(id) };
	}
	case C_TREFOILKNOT:
	{
		const COMP_UID id = pTrefoiKnotPool.GetNewCompUID();
		return { id, pTrefoiKnotPool.AtPtr(id) };
	}
	default: return { 0 , nullptr };
	}
}

const COMP_UID ComponentsPool::GetNewComponentUID(ComponentType cType)
{
	switch (cType) {
	case C_TRANSFORM: return transPool.GetNewCompUID();
	case C_CAMERA: return camPool.GetNewCompUID();
	case C_MESH: return meshPool.GetNewCompUID();
	case C_LIGHT: return lightPool.GetNewCompUID();
	case C_WATER: return waterPool.GetNewCompUID();
	case C_PARTICLEEMITER: particleSPool.GetNewCompUID(); break;
		//case C_FUSTRUM: break;
	case C_GRID: return pGridPool.GetNewCompUID();
	case C_ROCK: return pRockPool.GetNewCompUID();
	case C_CUBE: return pCubePool.GetNewCompUID();
	case C_DODECAHEDRON: return pDodecahedronPool.GetNewCompUID();
	case C_TETRAHEDRON: return pTetrahedronPool.GetNewCompUID();
	case C_OCTOHEDRON: return pOctohedronPool.GetNewCompUID();
	case C_ICOSAHEDRON:  return pIcosahedronPool.GetNewCompUID();
	case C_POINT: return pPointPool.GetNewCompUID();
	case C_PLANE: return pPlanePool.GetNewCompUID();
	case C_SPHERE: return pSpherePool.GetNewCompUID();
	case C_CYLINDER: return pCylinderPool.GetNewCompUID();
	case C_HEMISHPERE: return pHemiSpherePool.GetNewCompUID();
	case C_TORUS:  return pTorusPool.GetNewCompUID();
	case C_TREFOILKNOT: return pTrefoiKnotPool.GetNewCompUID(); }

	return 0ull;
}

RE_Component* ComponentsPool::GetNewComponentPtr(ComponentType cType)
{
	RE_Component* ret = nullptr;
	switch (cType) {
	case C_TRANSFORM: ret = static_cast<RE_Component*>(transPool.AtPtr(transPool.GetNewCompUID())); break;
	case C_CAMERA: ret = static_cast<RE_Component*>(camPool.AtPtr(camPool.GetNewCompUID())); break;
	case C_MESH: ret = static_cast<RE_Component*>(meshPool.AtPtr(meshPool.GetNewCompUID())); break;
	case C_LIGHT: ret = static_cast<RE_Component*>(lightPool.AtPtr(lightPool.GetNewCompUID())); break;
	case C_WATER: ret = static_cast<RE_Component*>(waterPool.AtPtr(waterPool.GetNewCompUID())); break;
	case C_PARTICLEEMITER: ret = static_cast<RE_Component*>(particleSPool.AtPtr(particleSPool.GetNewCompUID())); break;
	case C_FUSTRUM: break;
	case C_GRID: ret = static_cast<RE_Component*>(pGridPool.AtPtr(pGridPool.GetNewCompUID())); break;
	case C_ROCK: ret = static_cast<RE_Component*>(pRockPool.AtPtr(pRockPool.GetNewCompUID())); break;
	case C_CUBE: ret = static_cast<RE_Component*>(pCubePool.AtPtr(pCubePool.GetNewCompUID())); break;
	case C_DODECAHEDRON: ret = static_cast<RE_Component*>(pDodecahedronPool.AtPtr(pDodecahedronPool.GetNewCompUID())); break;
	case C_TETRAHEDRON: ret = static_cast<RE_Component*>(pTetrahedronPool.AtPtr(pTetrahedronPool.GetNewCompUID())); break;
	case C_OCTOHEDRON: ret = static_cast<RE_Component*>(pOctohedronPool.AtPtr(pOctohedronPool.GetNewCompUID())); break;
	case C_ICOSAHEDRON: ret = static_cast<RE_Component*>(pIcosahedronPool.AtPtr(pIcosahedronPool.GetNewCompUID())); break;
	case C_POINT: ret = static_cast<RE_Component*>(pPointPool.AtPtr(pPointPool.GetNewCompUID())); break;
	case C_PLANE: ret = static_cast<RE_Component*>(pPlanePool.AtPtr(pPlanePool.GetNewCompUID())); break;
	case C_SPHERE: ret = static_cast<RE_Component*>(pSpherePool.AtPtr(pSpherePool.GetNewCompUID())); break;
	case C_CYLINDER: ret = static_cast<RE_Component*>(pCylinderPool.AtPtr(pCylinderPool.GetNewCompUID())); break;
	case C_HEMISHPERE: ret = static_cast<RE_Component*>(pHemiSpherePool.AtPtr(pHemiSpherePool.GetNewCompUID())); break;
	case C_TORUS: ret = static_cast<RE_Component*>(pTorusPool.AtPtr(pTorusPool.GetNewCompUID())); break;
	case C_TREFOILKNOT: ret = static_cast<RE_Component*>(pTrefoiKnotPool.AtPtr(pTrefoiKnotPool.GetNewCompUID())); break;
	}
	return ret;
}

RE_Component* ComponentsPool::CopyComponent(GameObjectsPool* pool, RE_Component* copy, GO_UID parent)
{
	RE_Component* ret = nullptr;
	ComponentType cType = copy->GetType();
	switch (cType) {
	case C_TRANSFORM: ret = static_cast<RE_Component*>(transPool.AtPtr(transPool.GetNewCompUID())); break;
	case C_CAMERA: ret = static_cast<RE_Component*>(camPool.AtPtr(camPool.GetNewCompUID())); break;
	case C_MESH: ret = static_cast<RE_Component*>(meshPool.AtPtr(meshPool.GetNewCompUID())); break;
	case C_LIGHT: ret = static_cast<RE_Component*>(lightPool.AtPtr(lightPool.GetNewCompUID())); break;
	case C_WATER: ret = static_cast<RE_Component*>(waterPool.AtPtr(waterPool.GetNewCompUID())); break;
	case C_PARTICLEEMITER: ret = static_cast<RE_Component*>(particleSPool.AtPtr(particleSPool.GetNewCompUID())); break;
	case C_FUSTRUM: break;
	case C_GRID: ret = static_cast<RE_Component*>(pGridPool.AtPtr(pGridPool.GetNewCompUID())); break;
	case C_ROCK: ret = static_cast<RE_Component*>(pRockPool.AtPtr(pRockPool.GetNewCompUID())); break;
	case C_CUBE: ret = static_cast<RE_Component*>(pCubePool.AtPtr(pCubePool.GetNewCompUID())); break;
	case C_DODECAHEDRON: ret = static_cast<RE_Component*>(pDodecahedronPool.AtPtr(pDodecahedronPool.GetNewCompUID())); break;
	case C_TETRAHEDRON: ret = static_cast<RE_Component*>(pTetrahedronPool.AtPtr(pTetrahedronPool.GetNewCompUID())); break;
	case C_OCTOHEDRON: ret = static_cast<RE_Component*>(pOctohedronPool.AtPtr(pOctohedronPool.GetNewCompUID())); break;
	case C_ICOSAHEDRON: ret = static_cast<RE_Component*>(pIcosahedronPool.AtPtr(pIcosahedronPool.GetNewCompUID())); break;
	case C_POINT: ret = static_cast<RE_Component*>(pPointPool.AtPtr(pPointPool.GetNewCompUID())); break;
	case C_PLANE: ret = static_cast<RE_Component*>(pPlanePool.AtPtr(pPlanePool.GetNewCompUID())); break;
	case C_SPHERE: ret = static_cast<RE_Component*>(pSpherePool.AtPtr(pSpherePool.GetNewCompUID())); break;
	case C_CYLINDER: ret = static_cast<RE_Component*>(pCylinderPool.AtPtr(pCylinderPool.GetNewCompUID())); break;
	case C_HEMISHPERE: ret = static_cast<RE_Component*>(pHemiSpherePool.AtPtr(pHemiSpherePool.GetNewCompUID())); break;
	case C_TORUS: ret = static_cast<RE_Component*>(pTorusPool.AtPtr(pTorusPool.GetNewCompUID())); break;
	case C_TREFOILKNOT: ret = static_cast<RE_Component*>(pTrefoiKnotPool.AtPtr(pTrefoiKnotPool.GetNewCompUID())); break; }

	ret->CopySetUp(pool, copy, parent);
	return ret;
}

void ComponentsPool::DestroyComponent(ComponentType cType, COMP_UID toDelete)
{
	RE_Component* cmp = GetComponentPtr(toDelete, cType);
	cmp->GetGOPtr()->ReleaseComponent(toDelete, cType);
	switch (cType) {
	case C_TRANSFORM: transPool.Pop(toDelete); break;
	case C_CAMERA: camPool.Pop(toDelete); break;
	case C_MESH: meshPool.Pop(toDelete); break;
	case C_LIGHT: lightPool.Pop(toDelete); break;
	case C_WATER: waterPool.Pop(toDelete); break;
	case C_PARTICLEEMITER: particleSPool.Pop(toDelete); break;
	case C_GRID: pGridPool.Pop(toDelete); break;
	case C_ROCK: pRockPool.Pop(toDelete); break;
	case C_CUBE: pCubePool.Pop(toDelete); break;
	case C_DODECAHEDRON: pDodecahedronPool.Pop(toDelete); break;
	case C_TETRAHEDRON: pTetrahedronPool.Pop(toDelete); break;
	case C_OCTOHEDRON: pOctohedronPool.Pop(toDelete); break;
	case C_ICOSAHEDRON: pIcosahedronPool.Pop(toDelete); break;
	case C_POINT:  pPointPool.Pop(toDelete); break;
	case C_PLANE:  pPlanePool.Pop(toDelete); break;
	case C_SPHERE: pSpherePool.Pop(toDelete); break;
	case C_CYLINDER: pCylinderPool.Pop(toDelete); break;
	case C_HEMISHPERE: pHemiSpherePool.Pop(toDelete); break;
	case C_TORUS: pTorusPool.Pop(toDelete); break;
	case C_TREFOILKNOT: pTrefoiKnotPool.Pop(toDelete); break; }
}

eastl::vector<COMP_UID> ComponentsPool::GetAllCompUID(ushortint type) const
{
	eastl::vector<COMP_UID> ret;
	switch (type) {
	case C_TRANSFORM: ret = transPool.GetAllKeys(); break;
	case C_CAMERA: ret = camPool.GetAllKeys(); break;
	case C_MESH: ret = meshPool.GetAllKeys(); break;
	case C_LIGHT: ret = lightPool.GetAllKeys(); break;
	case C_WATER: ret = waterPool.GetAllKeys(); break;
	case C_PARTICLEEMITER: ret = particleSPool.GetAllKeys(); break;
	case C_FUSTRUM: break;
	case C_GRID: ret = pGridPool.GetAllKeys(); break;
	case C_ROCK: ret = pRockPool.GetAllKeys(); break;
	case C_CUBE: ret = pCubePool.GetAllKeys(); break;
	case C_DODECAHEDRON: ret = pDodecahedronPool.GetAllKeys(); break;
	case C_TETRAHEDRON: ret = pTetrahedronPool.GetAllKeys(); break;
	case C_OCTOHEDRON: ret = pOctohedronPool.GetAllKeys(); break;
	case C_ICOSAHEDRON: ret = pIcosahedronPool.GetAllKeys(); break;
	case C_POINT: ret = pPointPool.GetAllKeys(); break;
	case C_PLANE: ret = pPlanePool.GetAllKeys(); break;
	case C_SPHERE: ret = pSpherePool.GetAllKeys(); break;
	case C_CYLINDER: ret = pCylinderPool.GetAllKeys(); break;
	case C_HEMISHPERE: ret = pHemiSpherePool.GetAllKeys(); break;
	case C_TORUS: ret = pTorusPool.GetAllKeys(); break;
	case C_TREFOILKNOT: ret = pTrefoiKnotPool.GetAllKeys(); break; }

	return ret;
}

eastl::vector<RE_Component*> ComponentsPool::GetAllCompPtr(ushortint type) const
{
	eastl::vector<RE_Component*> ret;
	switch (type)
	{
	case C_TRANSFORM:
	{
		eastl::vector<COMP_UID> ids = transPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<RE_Component*>(transPool.AtPtr(id)));
		break;
	}
	case C_CAMERA:
	{
		eastl::vector<COMP_UID> ids = camPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<RE_Component*>(camPool.AtPtr(id)));
		break;
	}
	case C_MESH:
	{
		eastl::vector<COMP_UID> ids = meshPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<RE_Component*>(meshPool.AtPtr(id)));
		break;
	}
	case C_LIGHT:
	{
		eastl::vector<COMP_UID> ids = lightPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<RE_Component*>(lightPool.AtPtr(id)));
		break;
	}
	case C_WATER:
	{
		eastl::vector<COMP_UID> ids = waterPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<RE_Component*>(waterPool.AtPtr(id)));
		break;
	}
	case C_PARTICLEEMITER:
	{
		eastl::vector<COMP_UID> ids = particleSPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<RE_Component*>(particleSPool.AtPtr(id)));
		break;
	}
	case C_FUSTRUM: break;
	case C_GRID:
	{
		eastl::vector<COMP_UID> ids = pGridPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<RE_Component*>(pGridPool.AtPtr(id)));
		break;
	}
	case C_ROCK:
	{
		eastl::vector<COMP_UID> ids = pRockPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<RE_Component*>(pRockPool.AtPtr(id)));
		break;
	}
	case C_CUBE:
	{
		eastl::vector<COMP_UID> ids = pCubePool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<RE_Component*>(pCubePool.AtPtr(id)));
		break;
	}
	case C_DODECAHEDRON:
	{
		eastl::vector<COMP_UID> ids = pDodecahedronPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<RE_Component*>(pDodecahedronPool.AtPtr(id)));
		break;
	}
	case C_TETRAHEDRON:
	{
		eastl::vector<COMP_UID> ids = pTetrahedronPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<RE_Component*>(pTetrahedronPool.AtPtr(id)));
		break;
	}
	case C_OCTOHEDRON:
	{
		eastl::vector<COMP_UID> ids = pOctohedronPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<RE_Component*>(pOctohedronPool.AtPtr(id)));
		break;
	}
	case C_ICOSAHEDRON:
	{
		eastl::vector<COMP_UID> ids = pIcosahedronPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<RE_Component*>(pIcosahedronPool.AtPtr(id)));
		break;
	}
	case C_POINT:
	{
		eastl::vector<COMP_UID> ids = pPointPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<RE_Component*>(pPointPool.AtPtr(id)));
		break;
	}
	case C_PLANE:
	{
		eastl::vector<COMP_UID> ids = pPlanePool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<RE_Component*>(pPlanePool.AtPtr(id)));
		break;
	}
	case C_SPHERE:
	{
		eastl::vector<COMP_UID> ids = pSpherePool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<RE_Component*>(pSpherePool.AtPtr(id)));
		break;
	}
	case C_CYLINDER:
	{
		eastl::vector<COMP_UID> ids = pCylinderPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<RE_Component*>(pCylinderPool.AtPtr(id)));
		break;
	}
	case C_HEMISHPERE:
	{
		eastl::vector<COMP_UID> ids = pHemiSpherePool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<RE_Component*>(pHemiSpherePool.AtPtr(id)));
		break;
	}
	case C_TORUS:
	{
		eastl::vector<COMP_UID> ids = pTorusPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<RE_Component*>(pTorusPool.AtPtr(id)));
		break;
	}
	case C_TREFOILKNOT:
	{
		eastl::vector<COMP_UID> ids = pTrefoiKnotPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<RE_Component*>(pTrefoiKnotPool.AtPtr(id)));
		break;
	}
	}

	return ret;
}

eastl::vector<const RE_Component*> ComponentsPool::GetAllCompCPtr(ushortint type) const
{
	eastl::vector<const RE_Component*> ret;
	switch (type)
	{
	case C_TRANSFORM:
	{
		eastl::vector<COMP_UID> ids = transPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<const RE_Component*>(transPool.AtCPtr(id)));
		break;
	}
	case C_CAMERA:
	{
		eastl::vector<COMP_UID> ids = camPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<const RE_Component*>(camPool.AtCPtr(id)));
		break;
	}
	case C_MESH:
	{
		eastl::vector<COMP_UID> ids = meshPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<const RE_Component*>(meshPool.AtCPtr(id)));
		break;
	}
	case C_LIGHT:
	{
		eastl::vector<COMP_UID> ids = lightPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<const RE_Component*>(lightPool.AtCPtr(id)));
		break;
	}
	case C_WATER:
	{
		eastl::vector<COMP_UID> ids = waterPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<const RE_Component*>(waterPool.AtCPtr(id)));
		break;
	}
	case C_PARTICLEEMITER:
	{
		eastl::vector<COMP_UID> ids = particleSPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<const RE_Component*>(particleSPool.AtCPtr(id)));
		break;
	}
	case C_FUSTRUM: break;
	case C_GRID:
	{
		eastl::vector<COMP_UID> ids = pGridPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<const RE_Component*>(pGridPool.AtCPtr(id)));
		break;
	}
	case C_ROCK:
	{
		eastl::vector<COMP_UID> ids = pRockPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<const RE_Component*>(pRockPool.AtCPtr(id)));
		break;
	}
	case C_CUBE:
	{
		eastl::vector<COMP_UID> ids = pCubePool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<const RE_Component*>(pCubePool.AtCPtr(id)));
		break;
	}
	case C_DODECAHEDRON:
	{
		eastl::vector<COMP_UID> ids = pDodecahedronPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<const RE_Component*>(pDodecahedronPool.AtCPtr(id)));
		break;
	}
	case C_TETRAHEDRON:
	{
		eastl::vector<COMP_UID> ids = pTetrahedronPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<const RE_Component*>(pTetrahedronPool.AtCPtr(id)));
		break;
	}
	case C_OCTOHEDRON:
	{
		eastl::vector<COMP_UID> ids = pOctohedronPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<const RE_Component*>(pOctohedronPool.AtCPtr(id)));
		break;
	}
	case C_ICOSAHEDRON:
	{
		eastl::vector<COMP_UID> ids = pIcosahedronPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<const RE_Component*>(pIcosahedronPool.AtCPtr(id)));
		break;
	}
	case C_POINT:
	{
		eastl::vector<COMP_UID> ids = pPointPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<const RE_Component*>(pPointPool.AtCPtr(id)));
		break;
	}
	case C_PLANE:
	{
		eastl::vector<COMP_UID> ids = pPlanePool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<const RE_Component*>(pPlanePool.AtCPtr(id)));
		break;
	}
	case C_SPHERE:
	{
		eastl::vector<COMP_UID> ids = pSpherePool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<const RE_Component*>(pSpherePool.AtCPtr(id)));
		break;
	}
	case C_CYLINDER:
	{
		eastl::vector<COMP_UID> ids = pCylinderPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<const RE_Component*>(pCylinderPool.AtCPtr(id)));
		break;
	}
	case C_HEMISHPERE:
	{
		eastl::vector<COMP_UID> ids = pHemiSpherePool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<const RE_Component*>(pHemiSpherePool.AtCPtr(id)));
		break;
	}
	case C_TORUS:
	{
		eastl::vector<COMP_UID> ids = pTorusPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<const RE_Component*>(pTorusPool.AtCPtr(id)));
		break;
	}
	case C_TREFOILKNOT:
	{
		eastl::vector<COMP_UID> ids = pTrefoiKnotPool.GetAllKeys();
		for (auto id : ids) ret.push_back(static_cast<const RE_Component*>(pTrefoiKnotPool.AtCPtr(id)));
		break;
	}
	}

	return ret;
}

eastl::vector<eastl::pair<const COMP_UID, RE_Component*>> ComponentsPool::GetAllCompData(ushortint type) const
{
	eastl::vector<eastl::pair<const COMP_UID, RE_Component*>> ret;
	switch (type)
	{
	case C_TRANSFORM:
	{
		for (auto id : transPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(transPool.AtPtr(id)) });
		break;
	}
	case C_CAMERA:
	{
		for (auto id : camPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(camPool.AtPtr(id)) });
		break;
	}
	case C_MESH:
	{
		for (auto id : meshPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(meshPool.AtPtr(id)) });
		break;
	}
	case C_LIGHT:
	{
		for (auto id : lightPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(lightPool.AtPtr(id)) });
		break;
	}
	case C_WATER:
	{
		for (auto id : waterPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(waterPool.AtPtr(id)) });
		break;
	}
	case C_PARTICLEEMITER:
	{
		for (auto id : particleSPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(particleSPool.AtPtr(id)) });
		break;
	}
	case C_FUSTRUM: break;
	case C_GRID:
	{
		for (auto id : pGridPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pGridPool.AtPtr(id)) });
		break;
	}
	case C_ROCK:
	{
		for (auto id : pRockPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pRockPool.AtPtr(id)) });
		break;
	}
	case C_CUBE:
	{
		for (auto id : pCubePool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pCubePool.AtPtr(id)) });
		break;
	}
	case C_DODECAHEDRON:
	{
		for (auto id : pDodecahedronPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pDodecahedronPool.AtPtr(id)) });
		break;
	}
	case C_TETRAHEDRON:
	{
		for (auto id : pTetrahedronPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pTetrahedronPool.AtPtr(id)) });
		break;
	}
	case C_OCTOHEDRON:
	{
		for (auto id : pOctohedronPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pOctohedronPool.AtPtr(id)) });
		break;
	}
	case C_ICOSAHEDRON:
	{
		for (auto id : pIcosahedronPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pIcosahedronPool.AtPtr(id)) });
		break;
	}
	case C_POINT:
	{
		for (auto id : pPointPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pPointPool.AtPtr(id)) });
		break;
	}
	case C_PLANE:
	{
		for (auto id : pPlanePool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pPlanePool.AtPtr(id)) });
		break;
	}
	case C_SPHERE:
	{
		for (auto id : pSpherePool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pSpherePool.AtPtr(id)) });
		break;
	}
	case C_CYLINDER:
	{
		for (auto id : pCylinderPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pCylinderPool.AtPtr(id)) });
		break;
	}
	case C_HEMISHPERE:
	{
		for (auto id : pHemiSpherePool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pHemiSpherePool.AtPtr(id)) });
		break;
	}
	case C_TORUS:
	{
		for (auto id : pTorusPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pTorusPool.AtPtr(id)) });
		break;
	}
	case C_TREFOILKNOT:
	{
		for (auto id : pTrefoiKnotPool.GetAllKeys())
			ret.push_back({ id, static_cast<RE_Component*>(pTrefoiKnotPool.AtPtr(id)) });
		break;
	}
	}

	return ret;
}

unsigned int ComponentsPool::GetBinarySize() const
{
	uint size = 0;
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
	DEL(comps);
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
	DEL(comps);
}