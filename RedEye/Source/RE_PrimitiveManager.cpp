#include "RE_PrimitiveManager.h"

#include "Application.h"
#include "RE_ShaderImporter.h"
#include "RE_ResourceManager.h"
#include "RE_InternalResources.h"
#include "OutputLog.h"
#include "RE_Math.h"
#include "RE_CompPrimitive.h"
#include "RE_GLCacheManager.h"
#include "RE_Shader.h"

#include "SDL2/include/SDL.h"
#include "Glew/include/glew.h"
#include <gl/GL.h>
#include "par_shapes.h"

RE_PrimitiveManager::RE_PrimitiveManager()
{
	for (auto platonic : platonics) platonic = { 0, 0, 0, 0 };
}

RE_PrimitiveManager::~RE_PrimitiveManager()
{
	for (auto platonic : platonics)
	{
		if (platonic.triangles)
		{
			glDeleteVertexArrays(1, &platonic.vao);
			glDeleteBuffers(1, &platonic.vbo);
			glDeleteBuffers(1, &platonic.ebo);
		}
	}
}

void RE_PrimitiveManager::Init()
{
	App::ReportSoftware("par_shapes.h", nullptr, "https://github.com/prideout/par");
}

void RE_PrimitiveManager::SetUpComponentPrimitive(RE_CompPrimitive* cmpP)
{
	ComponentType type = cmpP->GetType();
	switch (type) {
		// Grid
	case C_GRID: dynamic_cast<RE_CompGrid*>(cmpP)->GridSetUp(); break;
		// Platonics
	case C_CUBE: dynamic_cast<RE_CompPlatonic*>(cmpP)->PlatonicSetUp(platonics[0].vao, platonics[0].triangles); break;
	case C_DODECAHEDRON: dynamic_cast<RE_CompPlatonic*>(cmpP)->PlatonicSetUp(platonics[1].vao, platonics[1].triangles); break;
	case C_TETRAHEDRON: dynamic_cast<RE_CompPlatonic*>(cmpP)->PlatonicSetUp(platonics[2].vao, platonics[2].triangles); break;
	case C_OCTOHEDRON: dynamic_cast<RE_CompPlatonic*>(cmpP)->PlatonicSetUp(platonics[3].vao, platonics[3].triangles); break;
	case C_ICOSAHEDRON: dynamic_cast<RE_CompPlatonic*>(cmpP)->PlatonicSetUp(platonics[4].vao, platonics[4].triangles); break;
		// Parametrics
	case C_PLANE: dynamic_cast<RE_CompParametric*>(cmpP)->ParametricSetUp(3, 3); break;
	case C_FUSTRUM: break;
	case C_SPHERE: dynamic_cast<RE_CompParametric*>(cmpP)->ParametricSetUp(16, 18); break;
	case C_CYLINDER: dynamic_cast<RE_CompParametric*>(cmpP)->ParametricSetUp(30, 3); break;
	case C_HEMISHPERE: dynamic_cast<RE_CompParametric*>(cmpP)->ParametricSetUp(10, 10); break;
	case C_TORUS: dynamic_cast<RE_CompParametric*>(cmpP)->ParametricSetUp(30, 40, 0.1f); break;
	case C_TREFOILKNOT: dynamic_cast<RE_CompParametric*>(cmpP)->ParametricSetUp(30, 40, 0.5f); break;
		// Rock
	case C_ROCK: dynamic_cast<RE_CompRock*>(cmpP)->RockSetUp(5, 20); break; }
}

eastl::pair<unsigned int, unsigned int> RE_PrimitiveManager::GetPlatonicData(unsigned short type)
{
	unsigned short index = type - C_CUBE;

	if (!platonics[index].vao)
	{
		par_shapes_mesh* mesh = nullptr;
		switch (type) {
		case C_CUBE: mesh = par_shapes_create_cube(); break;
		case C_DODECAHEDRON: mesh = par_shapes_create_dodecahedron(); break;
		case C_TETRAHEDRON: mesh = par_shapes_create_tetrahedron(); break;
		case C_OCTOHEDRON: mesh = par_shapes_create_octahedron(); break;
		case C_ICOSAHEDRON: mesh = par_shapes_create_icosahedron(); break; }

		UploadPlatonic(mesh, &platonics[index].vao, &platonics[index].vbo, &platonics[index].ebo, &platonics[index].triangles);
		par_shapes_free_mesh(mesh);
	}
	
	return { platonics[index].vao, platonics[index].triangles };
}

RE_PrimitiveManager::PlatonicData RE_PrimitiveManager::CreateSphere(int slices, int stacks)
{
	if (slices < 3) slices = 3;
	if (stacks < 3) stacks = 3;

	PlatonicData ret;

	par_shapes_mesh* sphere = par_shapes_create_parametric_sphere(slices, stacks);

	float* points = new float[sphere->npoints * 3];
	float* normals = new float[sphere->npoints * 3];
	float* texCoords = new float[sphere->npoints * 2];

	uint meshSize = 0;
	size_t size = sphere->npoints * 3 * sizeof(float);
	uint stride = 0;

	memcpy(points, sphere->points, size);
	meshSize += 3 * sphere->npoints;
	stride += 3;

	memcpy(normals, sphere->normals, size);
	meshSize += 3 * sphere->npoints;
	stride += 3;

	size = sphere->npoints * 2 * sizeof(float);
	memcpy(texCoords, sphere->tcoords, size);
	meshSize += 2 * sphere->npoints;
	stride += 2;

	stride *= sizeof(float);
	float* meshBuffer = new float[meshSize];
	float* cursor = meshBuffer;
	for (int i = 0; i < sphere->npoints; i++)
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

	glGenVertexArrays(1, &ret.vao);
	RE_GLCacheManager::ChangeVAO(ret.vao);

	glGenBuffers(1, &ret.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, ret.vbo);
	glBufferData(GL_ARRAY_BUFFER, meshSize * sizeof(float), meshBuffer, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(sizeof(float) * 3u));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(sizeof(float) * 6u));

	glGenBuffers(1, &ret.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ret.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere->ntriangles * sizeof(unsigned short) * 3, sphere->triangles, GL_STATIC_DRAW);

	ret.triangles = sphere->ntriangles;

	par_shapes_free_mesh(sphere);
	DEL_A(points);
	DEL_A(normals);
	DEL_A(texCoords);
	DEL_A(meshBuffer);

	return ret;
}

void RE_PrimitiveManager::UploadPlatonic(par_shapes_mesh_s* plato, unsigned int* vao, unsigned int* vbo, unsigned int* ebo, unsigned int* triangles)
{
	*triangles = static_cast<unsigned int>(plato->ntriangles);

	par_shapes_unweld(plato, true);
	par_shapes_compute_normals(plato);

	float* points = new float[plato->npoints * 3];
	float* normals = new float[plato->npoints * 3];

	uint meshSize = 0;
	size_t size = plato->npoints * 3 * sizeof(float);
	uint stride = 0;

	memcpy(points, plato->points, size);
	meshSize += 3 * plato->npoints;
	stride += 3;

	memcpy(normals, plato->normals, size);
	meshSize += 3 * plato->npoints;
	stride += 3;

	stride *= sizeof(float);
	float* meshBuffer = new float[meshSize];
	float* cursor = meshBuffer;
	for (int i = 0; i < plato->npoints; i++)
	{
		uint cursorSize = 3;
		size_t size = sizeof(float) * 3;

		memcpy(cursor, &points[i * 3], size);
		cursor += cursorSize;

		memcpy(cursor, &normals[i * 3], size);
		cursor += cursorSize;
	}

	glGenVertexArrays(1, vao);
	RE_GLCacheManager::ChangeVAO(*vao);

	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, meshSize * sizeof(float), meshBuffer, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(sizeof(float) * 3));

	glGenBuffers(1, ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, *triangles * sizeof(unsigned short) * 3u, plato->triangles, GL_STATIC_DRAW);
	
	DEL_A(points);
	DEL_A(normals);
	DEL_A(meshBuffer);
}