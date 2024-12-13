#include <MGL/Math/float3.h>
#include <EASTL/map.h>
#include <EASTL/utility.h>
#include <EASTL/array.h>
#include <EASTL/bit.h>

#include "RE_PrimitiveManager.h"

#include "RE_Memory.h"
#include "Application.h"
#include "RE_CompPrimitive.h"
#include "RE_GLCache.h"

#include <par_shapes.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <gl/GL.h>

RE_PrimitiveManager::RE_PrimitiveManager()
{
	platonics = new PrimData[6];
}

RE_PrimitiveManager::~RE_PrimitiveManager()
{
	DEL(platonics)
}

void RE_PrimitiveManager::Init()
{
	RE_SOFT_NS("par_shapes.h", "https://github.com/prideout/par");
}

void RE_PrimitiveManager::Clear() { }

void RE_PrimitiveManager::SetUpComponentPrimitive(RE_CompPrimitive* cmpP)
{
	switch (cmpP->GetType())
	{
		// Grid
	case RE_Component::Type::GRID: dynamic_cast<RE_CompGrid*>(cmpP)->GridSetUp(50); break;
		// Platonics
	case RE_Component::Type::POINT: dynamic_cast<RE_CompPlatonic*>(cmpP)->PlatonicSetUp(); break;
	case RE_Component::Type::CUBE: dynamic_cast<RE_CompPlatonic*>(cmpP)->PlatonicSetUp(); break;
	case RE_Component::Type::DODECAHEDRON: dynamic_cast<RE_CompPlatonic*>(cmpP)->PlatonicSetUp(); break;
	case RE_Component::Type::TETRAHEDRON: dynamic_cast<RE_CompPlatonic*>(cmpP)->PlatonicSetUp(); break;
	case RE_Component::Type::OCTOHEDRON: dynamic_cast<RE_CompPlatonic*>(cmpP)->PlatonicSetUp(); break;
	case RE_Component::Type::ICOSAHEDRON: dynamic_cast<RE_CompPlatonic*>(cmpP)->PlatonicSetUp(); break;
		// Parametrics
	case RE_Component::Type::PLANE: dynamic_cast<RE_CompParametric*>(cmpP)->ParametricSetUp(3, 3); break;
	case RE_Component::Type::SPHERE: dynamic_cast<RE_CompParametric*>(cmpP)->ParametricSetUp(16, 18); break;
	case RE_Component::Type::CYLINDER: dynamic_cast<RE_CompParametric*>(cmpP)->ParametricSetUp(30, 3); break;
	case RE_Component::Type::HEMISHPERE: dynamic_cast<RE_CompParametric*>(cmpP)->ParametricSetUp(10, 10); break;
	case RE_Component::Type::TORUS: dynamic_cast<RE_CompParametric*>(cmpP)->ParametricSetUp(30, 40, 0.1f); break;
	case RE_Component::Type::TREFOILKNOT: dynamic_cast<RE_CompParametric*>(cmpP)->ParametricSetUp(30, 40, 0.5f); break;
		// Rock
	case RE_Component::Type::ROCK: dynamic_cast<RE_CompRock*>(cmpP)->RockSetUp(5, 20); break;
	default: break;
	}
}

eastl::pair<unsigned int, unsigned int> RE_PrimitiveManager::GetPrimitiveMeshData(RE_Component* primComp, int id)
{
	RE_Component::Type pType = primComp->GetType();
	switch (pType)
	{
	case RE_Component::Type::CUBE:
	case RE_Component::Type::POINT:
	case RE_Component::Type::DODECAHEDRON:
	case RE_Component::Type::TETRAHEDRON:
	case RE_Component::Type::OCTOHEDRON:
	case RE_Component::Type::ICOSAHEDRON:
	{
		unsigned short index = static_cast<ushort>(pType) - static_cast<ushort>(RE_Component::Type::CUBE);

		if (!platonics[index].refCount)
		{
			if (pType == RE_Component::Type::POINT) GeneratePoint(platonics[index]);
			else
			{
				par_shapes_mesh* mesh = nullptr;
				switch (pType)
				{
				case RE_Component::Type::CUBE: mesh = par_shapes_create_cube(); break;
				case RE_Component::Type::DODECAHEDRON: mesh = par_shapes_create_dodecahedron(); break;
				case RE_Component::Type::TETRAHEDRON: mesh = par_shapes_create_tetrahedron(); break;
				case RE_Component::Type::OCTOHEDRON: mesh = par_shapes_create_octahedron(); break;
				case RE_Component::Type::ICOSAHEDRON: mesh = par_shapes_create_icosahedron(); break;
				default: break;
				}

				UploadPlatonic(platonics[index], mesh);
				par_shapes_free_mesh(mesh);
			}
		}
		platonics[index].refCount += 1;
		return { platonics[index].vao, platonics[index].triangles };
	}
	}

	auto pIter = primReference.find(pType);
	if (pIter != primReference.end())
	{
		auto iIter = pIter->second.find(id);
		if (iIter != pIter->second.end())
		{
			iIter->second.refCount += 1;
			return { iIter->second.vao, iIter->second.triangles };
		}
	}

	if (pIter == primReference.end())primReference.insert({ pType, {} });
	primReference.at(pType).insert({ id, PrimData() });

	PrimData& newPrim = primReference.at(pType).at(id);
	newPrim.refCount += 1;

	par_shapes_mesh* pMesh = nullptr;
	RE_CompParametric* cP = nullptr;
	switch (pType)
	{
	case RE_Component::Type::GRID:
		GenerateGrid(newPrim, dynamic_cast<RE_CompGrid*>(primComp));
		return { newPrim.vao, newPrim.triangles };
	case RE_Component::Type::ROCK:
		GenerateRock(newPrim, dynamic_cast<RE_CompRock*>(primComp));
		return { newPrim.vao, newPrim.triangles };
	case RE_Component::Type::PLANE:
		cP = dynamic_cast<RE_CompParametric*>(primComp);
		pMesh = par_shapes_create_plane(cP->slices, cP->stacks);
		break;
	case RE_Component::Type::SPHERE:
		cP = dynamic_cast<RE_CompParametric*>(primComp);
		pMesh = par_shapes_create_parametric_sphere(cP->slices, cP->stacks);
		break;
	case RE_Component::Type::CYLINDER:
		cP = dynamic_cast<RE_CompParametric*>(primComp);
		pMesh = par_shapes_create_cylinder(cP->slices, cP->stacks);
		break;
	case RE_Component::Type::HEMISHPERE:
		cP = dynamic_cast<RE_CompParametric*>(primComp);
		pMesh = par_shapes_create_hemisphere(cP->slices, cP->stacks);
		break;
	case RE_Component::Type::TORUS:
		cP = dynamic_cast<RE_CompParametric*>(primComp);
		pMesh = par_shapes_create_torus(cP->slices, cP->stacks, cP->radius);
		break;
	case RE_Component::Type::TREFOILKNOT:
		cP = dynamic_cast<RE_CompParametric*>(primComp);
		pMesh = par_shapes_create_trefoil_knot(cP->slices, cP->stacks, cP->radius);
		break;
	}

	UploadParametric(newPrim, pMesh);
	par_shapes_free_mesh(pMesh);

	return { newPrim.vao, newPrim.triangles };
}

void RE_PrimitiveManager::UnUsePrimitive(RE_Component::Type pType, int id)
{
	switch (pType)
	{
	case RE_Component::Type::CUBE:
	case RE_Component::Type::POINT:
	case RE_Component::Type::DODECAHEDRON:
	case RE_Component::Type::TETRAHEDRON:
	case RE_Component::Type::OCTOHEDRON:
	case RE_Component::Type::ICOSAHEDRON:
	{
		ushort index = static_cast<ushort>(pType) - static_cast<ushort>(RE_Component::Type::CUBE);
		platonics[index].refCount -= 1;

		if (platonics[index].refCount == 0) {
			glDeleteVertexArrays(1, &platonics[index].vao);
			glDeleteBuffers(1, &platonics[index].vbo);
			glDeleteBuffers(1, &platonics[index].ebo);
		}
		return;
	}
	}

	PrimData& primD = primReference.at(pType).at(id);
	primD.refCount -= 1;
	if (primD.refCount == 0) {
		if(primD.vao) glDeleteVertexArrays(1, &primD.vao);
		if (primD.vbo) glDeleteBuffers(1, &primD.vbo);
		if (primD.ebo) glDeleteBuffers(1, &primD.ebo);

		primReference.at(pType).erase(id);
	}
}

void RE_PrimitiveManager::CreateSphere(int slices, int stacks, unsigned int& vao, unsigned int& vbo, unsigned int& ebo, unsigned int& triangles)
{
	if (slices < 3) slices = 3;
	if (stacks < 3) stacks = 3;

	par_shapes_mesh* sphere = par_shapes_create_parametric_sphere(slices, stacks);

	size_t vertex_count = sphere->npoints;
	float* points = new float[vertex_count * 3];
	float* normals = new float[vertex_count * 3];
	float* texCoords = new float[vertex_count * 2];

	size_t meshSize = 0;
	size_t size = vertex_count * 3 * sizeof(float);
	uint stride = 0;

	memcpy(points, sphere->points, size);
	meshSize += 3 * vertex_count;
	stride += 3;

	memcpy(normals, sphere->normals, size);
	meshSize += 3 * vertex_count;
	stride += 3;

	size = vertex_count * 2 * sizeof(float);
	memcpy(texCoords, sphere->tcoords, size);
	meshSize += 2 * vertex_count;
	stride += 2;

	stride *= sizeof(float);
	float* meshBuffer = new float[meshSize];
	float* cursor = meshBuffer;
	for (size_t i = 0; i < sphere->npoints; i++)
	{
		uint cursorSize = 3;
		size_t size = sizeof(float) * 3;

		memcpy(cursor, &points[i * 3], size);
		cursor += cursorSize;

		memcpy(cursor, &normals[i * 3], size);
		cursor += cursorSize;

		cursorSize = 2;
		size = sizeof(float) * 2;
		memcpy(cursor, &texCoords[i * 2], size);
		cursor += cursorSize;
	}

	glGenVertexArrays(1, &vao);
	RE_GLCache::ChangeVAO(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, meshSize * sizeof(float), meshBuffer, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(sizeof(float) * 3));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(sizeof(float) * 6));

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere->ntriangles * sizeof(ushort) * 3, sphere->triangles, GL_STATIC_DRAW);

	RE_GLCache::ChangeVAO(0);

	triangles = sphere->ntriangles;

	par_shapes_free_mesh(sphere);
	DEL_A(points);
	DEL_A(normals);
	DEL_A(texCoords);
	DEL_A(meshBuffer);
}

void RE_PrimitiveManager::GeneratePoint(PrimData& prim)
{
	glGenVertexArrays(1, &prim.vao);
	glGenBuffers(1, &prim.vbo);
	glGenBuffers(1, &prim.ebo);

	RE_GLCache::ChangeVAO(prim.vao);
	glBindBuffer(GL_ARRAY_BUFFER, prim.vbo);

	eastl::array<float, 9> triangle =
	{
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f,  0.5f, 0.0f
	};

	unsigned short index[3] = { 0, 1, 2 };

	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), &triangle[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(unsigned short), &index[0], GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, NULL);

	prim.triangles = 1;

	RE_GLCache::ChangeVAO(0);
}

void RE_PrimitiveManager::GenerateGrid(PrimData& prim, RE_CompGrid* gridC)
{
	eastl::vector<float> vertices;
	float d = static_cast<float>(gridC->divisions);
	float distance = gridC->distance = d * 2.5f;
	float f = 0.f;
	for (; f < d; f += 0.5f)
	{
		vertices.push_back((f * 5.f) - distance);
		vertices.push_back(0.f);
		vertices.push_back(distance);
		vertices.push_back((f * 5.f) - distance);
		vertices.push_back(0.f);
		vertices.push_back(-distance);
		vertices.push_back(distance);
		vertices.push_back(0.f);
		vertices.push_back((f * 5.f) - distance);
		vertices.push_back(-distance);
		vertices.push_back(0.f);
		vertices.push_back((f * 5.f) - distance);
	}

	vertices.push_back((f * 5.f) - distance);
	vertices.push_back(0.f);
	vertices.push_back(distance);
	vertices.push_back((f * 5.f) - distance);
	vertices.push_back(0.f);
	vertices.push_back(-distance);
	vertices.push_back(distance);
	vertices.push_back(0.f);
	vertices.push_back((f * 5.f) - distance);
	vertices.push_back(-distance);
	vertices.push_back(0.f);
	vertices.push_back((f * 5.f) - distance);

	glGenVertexArrays(1, &prim.vao);
	glGenBuffers(1, &prim.vbo);

	RE_GLCache::ChangeVAO(prim.vao);

	glBindBuffer(GL_ARRAY_BUFFER, prim.vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	RE_GLCache::ChangeVAO(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void RE_PrimitiveManager::GenerateRock(PrimData& prim, RE_CompRock* rockC)
{
	par_shapes_mesh* rock = par_shapes_create_rock(rockC->seed, rockC->nsubdivisions);

	size_t vertex_count = rock->npoints;

	float* points = new float[vertex_count * 3];
	float* normals = new float[vertex_count * 3];

	size_t meshSize = 0;
	size_t size = vertex_count * 3 * sizeof(float);
	uint stride = 0;

	memcpy(points, rock->points, size);
	meshSize += 3 * vertex_count;
	stride += 3;

	memcpy(normals, rock->normals, size);
	meshSize += 3 * vertex_count;
	stride += 3;

	stride *= sizeof(float);
	float* meshBuffer = new float[meshSize];
	float* cursor = meshBuffer;
	for (size_t i = 0; i < vertex_count; i++)
	{
		uint cursorSize = 3;
		size_t size = sizeof(float) * 3;

		memcpy(cursor, &points[i * 3], size);
		cursor += cursorSize;

		memcpy(cursor, &normals[i * 3], size);
		cursor += cursorSize;
	}

	glGenVertexArrays(1, &prim.vao);
	RE_GLCache::ChangeVAO(prim.vao);

	glGenBuffers(1, &prim.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, prim.vbo);
	glBufferData(GL_ARRAY_BUFFER, meshSize * sizeof(float), meshBuffer, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, eastl::bit_cast<void*>(sizeof(float) * 3));

	glGenBuffers(1, &prim.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, rock->ntriangles * sizeof(ushort) * 3, rock->triangles, GL_STATIC_DRAW);

	RE_GLCache::ChangeVAO(0);

	prim.triangles = rock->ntriangles;

	par_shapes_free_mesh(rock);
	DEL_A(points);
	DEL_A(normals);
	DEL_A(meshBuffer);
}

void RE_PrimitiveManager::UploadPlatonic(PrimData& prim, par_shapes_mesh_s* plato)
{
	prim.triangles = static_cast<uint>(plato->ntriangles);

	par_shapes_unweld(plato, true);
	par_shapes_compute_normals(plato);

	size_t vertex_count = plato->npoints;

	float* points = new float[vertex_count * 3];
	float* normals = new float[vertex_count * 3];

	size_t meshSize = 0;
	size_t size = vertex_count * 3 * sizeof(float);
	uint stride = 0;

	memcpy(points, plato->points, size);
	meshSize += 3 * vertex_count;
	stride += 3;

	memcpy(normals, plato->normals, size);
	meshSize += 3 * vertex_count;
	stride += 3;

	stride *= sizeof(float);
	float* meshBuffer = new float[meshSize];
	float* cursor = meshBuffer;
	for (size_t i = 0; i < vertex_count; i++)
	{
		uint cursorSize = 3;
		size_t size = sizeof(float) * 3;

		memcpy(cursor, &points[i * 3], size);
		cursor += cursorSize;

		memcpy(cursor, &normals[i * 3], size);
		cursor += cursorSize;
	}

	glGenVertexArrays(1, &prim.vao);
	RE_GLCache::ChangeVAO(prim.vao);

	glGenBuffers(1, &prim.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, prim.vbo);
	glBufferData(GL_ARRAY_BUFFER, meshSize * sizeof(float), meshBuffer, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(sizeof(float) * 3));

	glGenBuffers(1, &prim.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, prim.triangles * sizeof(unsigned short) * 3u, plato->triangles, GL_STATIC_DRAW);

	RE_GLCache::ChangeVAO(0);
	
	DEL_A(points);
	DEL_A(normals);
	DEL_A(meshBuffer);
}

void RE_PrimitiveManager::UploadParametric(PrimData& prim, par_shapes_mesh_s* param)
{
	prim.triangles = param->ntriangles;

	size_t vertex_count = param->npoints;

	float* points = new float[vertex_count * 3];
	float* normals = new float[vertex_count * 3];
	float* texCoords = new float[vertex_count * 2];

	size_t meshSize = 0;
	size_t size = vertex_count * 3 * sizeof(float);
	uint stride = 0;

	memcpy(points, param->points, size);
	meshSize += 3 * vertex_count;
	stride += 3;

	memcpy(normals, param->normals, size);
	meshSize += 3 * vertex_count;
	stride += 3;

	size = vertex_count * 2 * sizeof(float);
	memcpy(texCoords, param->tcoords, size);
	meshSize += 2 * vertex_count;
	stride += 2;

	stride *= sizeof(float);
	float* meshBuffer = new float[meshSize];
	float* cursor = meshBuffer;

	for (size_t i = 0; i < vertex_count; i++)
	{
		uint cursorSize = 3;
		size_t size = sizeof(float) * 3;

		memcpy(cursor, &points[i * 3], size);
		cursor += cursorSize;

		memcpy(cursor, &normals[i * 3], size);
		cursor += cursorSize;

		cursorSize = 2;
		size = sizeof(float) * 2;
		memcpy(cursor, &texCoords[i * 2], size);
		cursor += cursorSize;
	}

	glGenVertexArrays(1, &prim.vao);
	RE_GLCache::ChangeVAO(prim.vao);

	glGenBuffers(1, &prim.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, prim.vbo);
	glBufferData(GL_ARRAY_BUFFER, meshSize * sizeof(float), meshBuffer, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(sizeof(float) * 3u));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(sizeof(float) * 6u));

	glGenBuffers(1, &prim.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, prim.triangles * sizeof(unsigned short) * 3u, param->triangles, GL_STATIC_DRAW);

	RE_GLCache::ChangeVAO(0);

	DEL_A(points);
	DEL_A(normals);
	DEL_A(texCoords);
	DEL_A(meshBuffer);
}