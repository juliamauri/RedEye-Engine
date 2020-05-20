#include "RE_PrimitiveManager.h"

#include "Application.h"
#include "RE_ShaderImporter.h"
#include "RE_ResourceManager.h"
#include "RE_InternalResources.h"
#include "OutputLog.h"
#include "RE_Math.h"

#include "RE_CompPrimitive.h"

#include "RE_GLCache.h"
#include "RE_Shader.h"

#include "SDL2/include/SDL.h"
#include "Glew/include/glew.h"
#include <gl/GL.h>

#include "par_shapes.h"

#define CUBE_TRIANGLES 36

RE_PrimitiveManager::RE_PrimitiveManager()
{ }

RE_PrimitiveManager::~RE_PrimitiveManager()
{
	glDeleteVertexArrays(1, &(GLuint)vao_grid);
	glDeleteBuffers(1, &(GLuint)vbo_grid);
	glDeleteVertexArrays(1, &(GLuint)vao_cube);
	glDeleteBuffers(1, &(GLuint)vbo_cube);
	glDeleteBuffers(1, &(GLuint)ebo_cube);
	glDeleteVertexArrays(1, &(GLuint)vao_dodo);
	glDeleteBuffers(1, &(GLuint)vbo_dodo);
	glDeleteBuffers(1, &(GLuint)ebo_dodo);
	glDeleteVertexArrays(1, &(GLuint)vao_tetra);
	glDeleteBuffers(1, &(GLuint)vbo_tetra);
	glDeleteBuffers(1, &(GLuint)ebo_tetra);
	glDeleteVertexArrays(1, &(GLuint)vao_octo);
	glDeleteBuffers(1, &(GLuint)vbo_octo);
	glDeleteBuffers(1, &(GLuint)ebo_octo);
	glDeleteVertexArrays(1, &(GLuint)vao_icosa);
	glDeleteBuffers(1, &(GLuint)vbo_icosa);
	glDeleteBuffers(1, &(GLuint)ebo_icosa);
}

RE_CompPrimitive * RE_PrimitiveManager::CreateGrid(RE_GameObject* game_obj)
{
	if (vao_grid == 0) {

		eastl::vector<float> vertices;
		for (float f = 0.f; f < 50.f; f += 0.5f)
		{
			vertices.push_back((f * 5.f) - 125.f);
			vertices.push_back(0.f);
			vertices.push_back(125.f);
			vertices.push_back((f * 5.f) - 125.f);
			vertices.push_back(0.f);
			vertices.push_back(-125.f);
			vertices.push_back(125.f);
			vertices.push_back(0.f);
			vertices.push_back((f * 5.f) - 125.f);
			vertices.push_back(-125.f);
			vertices.push_back(0.f);
			vertices.push_back((f * 5.f) - 125.f);
		}

		glGenVertexArrays(1, &vao_grid);
		glGenBuffers(1, &vbo_grid);

		RE_GLCache::ChangeVAO(vao_grid);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_grid);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);

		RE_GLCache::ChangeVAO(vao_grid);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	RE_CompPrimitive* ret = new RE_CompGrid(game_obj, vao_grid, shaderPrimitive);
	return ret;
}

RE_CompPrimitive* RE_PrimitiveManager::CreateRock(RE_GameObject* game_obj, int seed, int nsubdivisions)
{
	return new RE_CompRock(game_obj, shaderPrimitive, seed, nsubdivisions);
}
RE_CompPrimitive* RE_PrimitiveManager::CreateCube(RE_GameObject* game_obj)
{
	return new RE_CompCube(game_obj, vao_cube, shaderPrimitive, cube_triangles);
}

RE_CompPrimitive* RE_PrimitiveManager::CreateDodecahedron(RE_GameObject* game_obj)
{
	return new RE_CompDodecahedron(game_obj, vao_dodo, shaderPrimitive, dodo_triangles);
}

RE_CompPrimitive* RE_PrimitiveManager::CreateTetrahedron(RE_GameObject* game_obj)
{
	return new RE_CompTetrahedron(game_obj, vao_tetra, shaderPrimitive, tetra_triangles);
}

RE_CompPrimitive* RE_PrimitiveManager::CreateOctohedron(RE_GameObject* game_obj)
{
	return new RE_CompOctohedron(game_obj, vao_octo, shaderPrimitive, octo_triangles);
}

RE_CompPrimitive* RE_PrimitiveManager::CreateIcosahedron(RE_GameObject* game_obj)
{
	return new RE_CompIcosahedron(game_obj, vao_icosa, shaderPrimitive, icosa_triangles);
}

RE_CompPrimitive* RE_PrimitiveManager::CreatePlane(RE_GameObject* game_obj, int slices, int stacks)
{
	return new RE_CompPlane(game_obj, shaderPrimitive, slices, stacks);
}

RE_CompPrimitive * RE_PrimitiveManager::CreateSphere(RE_GameObject* game_obj, int slices, int stacks)
{
	return new RE_CompSphere(game_obj, shaderPrimitive, slices, stacks);
}

RE_CompPrimitive * RE_PrimitiveManager::CreateCylinder(RE_GameObject* game_obj, int slices, int stacks)
{
	return new RE_CompCylinder(game_obj, shaderPrimitive, slices, stacks);
}

RE_CompPrimitive * RE_PrimitiveManager::CreateHemiSphere(RE_GameObject* game_obj, int slices, int stacks)
{
	return new RE_CompHemiSphere(game_obj, shaderPrimitive, slices, stacks);
}

RE_CompPrimitive* RE_PrimitiveManager::CreateTorus(RE_GameObject* game_obj, int slices, int stacks, float radius)
{
	return new RE_CompTorus(game_obj, shaderPrimitive, slices, stacks, radius);
}

RE_CompPrimitive* RE_PrimitiveManager::CreateTrefoilKnot(RE_GameObject* game_obj, int slices, int stacks, float radius)
{
	return new RE_CompTrefoiKnot(game_obj, shaderPrimitive, slices, stacks, radius);
}

unsigned int RE_PrimitiveManager::CheckCubeVAO()
{
	if (vao_cube == 0) {
		par_shapes_mesh* cube = par_shapes_create_cube();
		UploadPlatonic(cube, &vao_cube, &vbo_cube, &ebo_cube, &cube_triangles);
		par_shapes_free_mesh(cube);
	}
	return vao_cube;
}

unsigned int RE_PrimitiveManager::CheckDodecahedronVAO()
{
	if (vao_dodo == 0) {
		par_shapes_mesh* dodecahedron = par_shapes_create_dodecahedron();
		UploadPlatonic(dodecahedron, &vao_dodo, &vbo_dodo, &ebo_dodo, &dodo_triangles);
		par_shapes_free_mesh(dodecahedron);
	}
	return vao_dodo;
}

unsigned int RE_PrimitiveManager::CheckTetrahedronVAO()
{
	if (vao_tetra == 0) {
		par_shapes_mesh* tetrahedron = par_shapes_create_tetrahedron();
		UploadPlatonic(tetrahedron, &vao_tetra, &vbo_tetra, &ebo_tetra, &tetra_triangles);
		par_shapes_free_mesh(tetrahedron);
	}
	return vao_tetra;
}

unsigned int RE_PrimitiveManager::CheckOctohedronVAO()
{
	if (vao_octo == 0) {
		par_shapes_mesh* octohedron = par_shapes_create_octahedron();
		UploadPlatonic(octohedron, &vao_octo, &vbo_octo, &ebo_octo, &octo_triangles);
		par_shapes_free_mesh(octohedron);
	}
	return vao_octo;
}

unsigned int RE_PrimitiveManager::CheckIcosahedronVAO()
{
	if (vao_icosa == 0) {
		par_shapes_mesh* icosahedron = par_shapes_create_icosahedron();
		UploadPlatonic(icosahedron, &vao_icosa, &vbo_icosa, &ebo_icosa, &icosa_triangles);
		par_shapes_free_mesh(icosahedron);
	}
	return vao_icosa;
}

void RE_PrimitiveManager::UploadPlatonic(par_shapes_mesh_s* plato, unsigned int* vao, unsigned int* vbo, unsigned int* ebo, int* triangles)
{
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
	for (uint i = 0; i < plato->npoints; i++) {
		uint cursorSize = 3;
		size_t size = sizeof(float) * 3;

		memcpy(cursor, &points[i * 3], size);
		cursor += cursorSize;

		memcpy(cursor, &normals[i * 3], size);
		cursor += cursorSize;
	}


	glGenVertexArrays(1, vao);
	RE_GLCache::ChangeVAO(*vao);

	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, meshSize * sizeof(float), meshBuffer, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));

	glGenBuffers(1, ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, plato->ntriangles * sizeof(unsigned short) * 3, plato->triangles, GL_STATIC_DRAW);
	
	*triangles = plato->ntriangles;

	DEL_A(points);
	DEL_A(normals);
	DEL_A(meshBuffer);
}

bool RE_PrimitiveManager::Init(const char* def_shader)
{
	shaderPrimitive = ((RE_Shader*)App->resources->At(App->internalResources->GetDefaultShader()))->GetID();

	CheckCubeVAO();
	CheckDodecahedronVAO();
	CheckTetrahedronVAO();
	CheckOctohedronVAO();
	CheckIcosahedronVAO();

	App->ReportSoftware("par_shapes.h", nullptr, "https://github.com/prideout/par");

	return true;
}