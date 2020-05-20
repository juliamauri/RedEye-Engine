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
	glDeleteVertexArrays(1, &(GLuint)vao_cube);
	glDeleteBuffers(1, &(GLuint)vbo_cube);
	glDeleteBuffers(1, &(GLuint)ebo_cube);
}

RE_CompPrimitive * RE_PrimitiveManager::CreateGrid(RE_GameObject* game_obj)
{
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

	RE_CompPrimitive* ret = new RE_CompGrid(game_obj, vao_grid, shaderPrimitive);
	return ret;
}

RE_CompPrimitive* RE_PrimitiveManager::CreatePlane(RE_GameObject* game_obj, int slices, int stacks)
{
	return new RE_CompPlane(game_obj, shaderPrimitive, slices, stacks);
}

RE_CompPrimitive * RE_PrimitiveManager::CreateCube(RE_GameObject* game_obj)
{
	return new RE_CompCube(game_obj, vao_cube, shaderPrimitive, CUBE_TRIANGLES);
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

RE_CompPrimitive* RE_PrimitiveManager::CreateRock(RE_GameObject* game_obj, int seed, int nsubdivisions)
{
	return new RE_CompRock(game_obj, shaderPrimitive, seed, nsubdivisions);
}

unsigned int RE_PrimitiveManager::CheckCubeVAO()
{
	if (vao_cube == 0) {
		par_shapes_mesh* cube = par_shapes_create_cube();
		par_shapes_unweld(cube, true);
		par_shapes_compute_normals(cube);

		float* points = new float[cube->npoints * 3];
		float* normals = new float[cube->npoints * 3];
		//float* texCoords = new float[cube->npoints * 2];

		uint meshSize = 0;
		size_t size = cube->npoints * 3 * sizeof(float);
		uint stride = 0;

		memcpy(points, cube->points, size);
		meshSize += 3 * cube->npoints;
		stride += 3;

		memcpy(normals, cube->normals, size);
		meshSize += 3 * cube->npoints;
		stride += 3;

		//size = cube->npoints * 2 * sizeof(float);
		//memcpy(texCoords, cube->tcoords, size);
		//meshSize += 2 * cube->npoints;
		//stride += 2;

		stride *= sizeof(float);
		float* meshBuffer = new float[meshSize];
		float* cursor = meshBuffer;
		for (uint i = 0; i < cube->npoints; i++) {
			uint cursorSize = 3;
			size_t size = sizeof(float) * 3;

			memcpy(cursor, &points[i * 3], size);
			cursor += cursorSize;

			memcpy(cursor, &normals[i * 3], size);
			cursor += cursorSize;

			//cursorSize = 2;
			//size = sizeof(float) * 2;
			//memcpy(cursor, &texCoords[i * 2], size);
			//cursor += cursorSize;
		}


		glGenVertexArrays(1, &vao_cube);
		RE_GLCache::ChangeVAO(vao_cube);

		glGenBuffers(1, &vbo_cube);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_cube);
		glBufferData(GL_ARRAY_BUFFER, meshSize * sizeof(float), meshBuffer, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, NULL);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));

		//glEnableVertexAttribArray(4);
		//glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));

		glGenBuffers(1, &ebo_cube);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_cube);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube->ntriangles * sizeof(unsigned short) * 3, cube->triangles, GL_STATIC_DRAW);


		par_shapes_free_mesh(cube);
		DEL_A(points);
		DEL_A(normals);
		//DEL_A(texCoords);
		DEL_A(meshBuffer);

	}
	return vao_cube;
}

bool RE_PrimitiveManager::Init(const char* def_shader)
{
	
	shaderPrimitive = ((RE_Shader*)App->resources->At(App->internalResources->GetDefaultShader()))->GetID();

	CheckCubeVAO();

	App->ReportSoftware("par_shapes.h", nullptr, "https://github.com/prideout/par");

	return true;
}