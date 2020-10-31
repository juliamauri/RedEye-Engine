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

#define CUBE_TRIANGLES 36

RE_PrimitiveManager::RE_PrimitiveManager() {}

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

bool RE_PrimitiveManager::Init(const char* def_shader)
{
	CheckPlatonicVAO(C_CUBE);
	CheckPlatonicVAO(C_DODECAHEDRON);
	CheckPlatonicVAO(C_TETRAHEDRON);
	CheckPlatonicVAO(C_OCTOHEDRON);
	CheckPlatonicVAO(C_ICOSAHEDRON);

	App->ReportSoftware("par_shapes.h", nullptr, "https://github.com/prideout/par");

	return true;
}

void RE_PrimitiveManager::SetUpComponentPrimitive(RE_CompPrimitive* cmpP, RE_GameObject* parent)
{
	unsigned int shaderPrimitive = ((RE_Shader*)App->resources->At(App->internalResources->GetDefaultShader()))->GetID();
	switch (cmpP->GetType())
	{
	case C_CUBE:
		((RE_CompCube*)cmpP)->SetUp(parent, vao_cube, shaderPrimitive, cube_triangles);
		break;
	case C_DODECAHEDRON:
		((RE_CompDodecahedron*)cmpP)->SetUp(parent, vao_dodo, shaderPrimitive, dodo_triangles);
		break;
	case C_TETRAHEDRON:
		((RE_CompTetrahedron*)cmpP)->SetUp(parent, vao_tetra, shaderPrimitive, tetra_triangles);
		break;
	case C_OCTOHEDRON:
		((RE_CompOctohedron*)cmpP)->SetUp(parent, vao_octo, shaderPrimitive, octo_triangles);
		break;
	case C_ICOSAHEDRON:
		((RE_CompIcosahedron*)cmpP)->SetUp(parent, vao_icosa, shaderPrimitive, icosa_triangles);
		break;
	case C_SPHERE:
		((RE_CompSphere*)cmpP)->SetUp(parent, shaderPrimitive, 16, 18);
		break;
	case C_CYLINDER:
		((RE_CompCylinder*)cmpP)->SetUp(parent, shaderPrimitive, 30, 3);
		break;
	case C_HEMISHPERE:
		((RE_CompHemiSphere*)cmpP)->SetUp(parent, shaderPrimitive, 10, 10);
		break;
	case C_TORUS:
		((RE_CompTorus*)cmpP)->SetUp(parent, shaderPrimitive, 30, 40, true, 0.1f);
		break;
	case C_TREFOILKNOT:
		((RE_CompTrefoiKnot*)cmpP)->SetUp(parent, shaderPrimitive, 30, 40, true, 0.5f);
		break;
	case C_ROCK:
		((RE_CompRock*)cmpP)->SetUp(parent, shaderPrimitive, 5, 20);
		break;
	case C_PLANE:
		((RE_CompPlane*)cmpP)->SetUp(parent, shaderPrimitive, 3, 3);
		break;
	}
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

		RE_GLCacheManager::ChangeVAO(vao_grid);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_grid);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);

		RE_GLCacheManager::ChangeVAO(vao_grid);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	RE_CompPrimitive* ret = new RE_CompGrid(game_obj, vao_grid, ((RE_Shader*)App->resources->At(App->internalResources->GetDefaultShader()))->GetID());

	return ret;
}

unsigned int RE_PrimitiveManager::CheckPlatonicVAO(unsigned short type)
{
	unsigned int ret = 0;
	switch (type)
	{

	case C_CUBE:
		if (vao_cube == 0) {
			par_shapes_mesh* cube = par_shapes_create_cube();
			UploadPlatonic(cube, &vao_cube, &vbo_cube, &ebo_cube, &cube_triangles);
			par_shapes_free_mesh(cube);
		}
		ret = vao_cube;
		break;
	case C_DODECAHEDRON:
		if (vao_dodo == 0) {
			par_shapes_mesh* dodecahedron = par_shapes_create_dodecahedron();
			UploadPlatonic(dodecahedron, &vao_dodo, &vbo_dodo, &ebo_dodo, &dodo_triangles);
			par_shapes_free_mesh(dodecahedron);
		}
		ret = vao_dodo;
		break;
	case C_TETRAHEDRON:
		if (vao_tetra == 0) {
			par_shapes_mesh* tetrahedron = par_shapes_create_tetrahedron();
			UploadPlatonic(tetrahedron, &vao_tetra, &vbo_tetra, &ebo_tetra, &tetra_triangles);
			par_shapes_free_mesh(tetrahedron);
		}
		ret = vao_tetra;
		break;
	case C_OCTOHEDRON:
		if (vao_octo == 0) {
			par_shapes_mesh* octohedron = par_shapes_create_octahedron();
			UploadPlatonic(octohedron, &vao_octo, &vbo_octo, &ebo_octo, &octo_triangles);
			par_shapes_free_mesh(octohedron);
		}
		ret = vao_octo;
		break;
	case C_ICOSAHEDRON:
		if (vao_icosa == 0) {
			par_shapes_mesh* icosahedron = par_shapes_create_icosahedron();
			UploadPlatonic(icosahedron, &vao_icosa, &vbo_icosa, &ebo_icosa, &icosa_triangles);
			par_shapes_free_mesh(icosahedron);
		}
		ret = vao_icosa;
		break;
	}
	return ret;
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
	for (int i = 0; i < plato->npoints; i++) {
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
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));

	glGenBuffers(1, ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, plato->ntriangles * sizeof(unsigned short) * 3, plato->triangles, GL_STATIC_DRAW);
	
	*triangles = plato->ntriangles;

	DEL_A(points);
	DEL_A(normals);
	DEL_A(meshBuffer);
}