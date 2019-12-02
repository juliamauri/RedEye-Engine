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

#ifndef PAR_SHAPES_IMPLEMENTATION
#define PAR_SHAPES_IMPLEMENTATION
#endif

#include "par_shapes.h"


#define CUBE_TRIANGLES 36

RE_PrimitiveManager::RE_PrimitiveManager()
{
	primitives_count.insert(std::pair<ComponentType, unsigned int>(C_POINT, 0));
	primitives_count.insert(std::pair<ComponentType, unsigned int>(C_LINE, 0));
	primitives_count.insert(std::pair<ComponentType, unsigned int>(C_RAY, 0));
	primitives_count.insert(std::pair<ComponentType, unsigned int>(C_AXIS, 0));
	primitives_count.insert(std::pair<ComponentType, unsigned int>(C_TRIANGLE, 0));
	primitives_count.insert(std::pair<ComponentType, unsigned int>(C_PLANE, 0));
	primitives_count.insert(std::pair<ComponentType, unsigned int>(C_CUBE, 0));
	primitives_count.insert(std::pair<ComponentType, unsigned int>(C_FUSTRUM, 0));
	primitives_count.insert(std::pair<ComponentType, unsigned int>(C_SPHERE, 0));
	primitives_count.insert(std::pair<ComponentType, unsigned int>(C_CYLINDER, 0));
	primitives_count.insert(std::pair<ComponentType, unsigned int>(C_CAPSULE, 0));
}

RE_PrimitiveManager::~RE_PrimitiveManager()
{}

RE_CompPrimitive * RE_PrimitiveManager::CreateAxis(RE_GameObject* go)
{
	if(primitives_count.find(C_AXIS)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompAxis(go, vao_axis);
	return ret;
}

RE_CompPrimitive* RE_PrimitiveManager::CreatePoint(RE_GameObject* game_obj, math::vec pos)
{
	if (primitives_count.find(C_POINT)->second++ == 0)
	{
		math::vec p(0.0f, 0.0f, 0.0f);

		glGenVertexArrays(1, &vao_point);
		glGenBuffers(1, &vbo_point);

		RE_GLCache::ChangeVAO(vao_point);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_point);
		glBufferData(GL_ARRAY_BUFFER, sizeof(p), &p, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	RE_CompPoint* ret = new RE_CompPoint(game_obj, vao_point, shaderPrimitive, pos);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreateLine(RE_GameObject* game_obj, math::vec origin, math::vec end)
{
	if (primitives_count.find(C_LINE)->second++ == 0)
	{
		math::vec vecline[] = {
			origin,
			end
		};

		glGenVertexArrays(1, &vao_line);
		glGenBuffers(1, &vbo_line);

		RE_GLCache::ChangeVAO(vao_line);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_line);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vecline) * sizeof(math::vec), &vecline[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(vecline), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	RE_CompLine* ret = new RE_CompLine(game_obj, vao_line, shaderPrimitive,origin, end);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreateRay(RE_GameObject* game_obj)
{
	if (primitives_count.find(C_RAY)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompRay(game_obj, vao_ray);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreateTriangle(RE_GameObject* game_obj)
{
	if (primitives_count.find(C_TRIANGLE)->second++ == 0)
	{
		math::vec vPositionTriangle[] = {
			// positions       
			math::vec(1.0f, -1.0f, 0.0f),  // bottom right
			math::vec(-1.0f, -1.0f, 0.0f),   // bottom left
			math::vec(0.0f,  1.0f, 0.0f)      // top 
		};

		unsigned int index[] = { 0,1,2 };

		glGenVertexArrays(1, &vao_triangle);
		glGenBuffers(1, &vbo_triangle);
		glGenBuffers(1, &ebo_triangle);

		RE_GLCache::ChangeVAO(vao_triangle);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vPositionTriangle) * sizeof(math::vec), &vPositionTriangle[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_triangle);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(vPositionTriangle), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	}
	RE_CompTriangle* ret = new RE_CompTriangle(game_obj, vao_triangle, shaderPrimitive);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreatePlane(RE_GameObject* game_obj)
{
	if (primitives_count.find(C_PLANE)->second++ == 0)
	{
		std::vector<float> vertices;
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

		glGenVertexArrays(1, &vao_plane);
		glGenBuffers(1, &vbo_plane);

		RE_GLCache::ChangeVAO(vao_plane);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_plane);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// 2 triangle plane
		/*math::vec vPositionPlane[] = {
			// positions       
			math::vec(1.0f,  1.0f, 0.0f),  // top right
			math::vec(1.0f, -1.0f, 0.0f),  // bottom right
			math::vec(-1.0f, -1.0f, 0.0f), // bottom left
			math::vec(-1.0f,  1.0f, 0.0f) // top left
		};

		unsigned int index[] = { 0,1,3,1,2,3 };

		glGenVertexArrays(1, &vao_plane);
		glGenBuffers(1, &vbo_plane);
		glGenBuffers(1, &ebo_plane);

		RE_GLCache::ChangeVAO(vao_plane);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_plane);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vPositionPlane) * sizeof(math::vec), &vPositionPlane[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_plane);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(vPositionPlane), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);*/
	}
	RE_CompPrimitive* ret = new RE_CompPlane(game_obj, vao_plane, shaderPrimitive);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreateCube(RE_GameObject* game_obj)
{
	if (primitives_count.find(C_CUBE)->second++ == 0) {

		par_shapes_mesh* cube = par_shapes_create_cube();

		glGenVertexArrays(1, &vao_cube);
		glGenBuffers(1, &vbo_cube);

		RE_GLCache::ChangeVAO(vao_cube);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_cube);
		glBufferData(GL_ARRAY_BUFFER, cube->npoints * sizeof(float) * 3, cube->points, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &ebo_cube);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_cube);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube->ntriangles * sizeof(unsigned short) * 3, cube->triangles, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		par_shapes_free_mesh(cube);
	}
	RE_CompCube* ret = new RE_CompCube(game_obj, vao_cube, shaderPrimitive, CUBE_TRIANGLES);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreateFustrum(RE_GameObject* game_obj)
{
	if (primitives_count.find(C_FUSTRUM)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompFustrum(game_obj, vao_fustrum, shaderPrimitive);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreateSphere(RE_GameObject* game_obj, int slices, int stacks)
{
	return new RE_CompSphere(game_obj, shaderPrimitive, slices, stacks);
}

RE_CompPrimitive * RE_PrimitiveManager::CreateCylinder(RE_GameObject* game_obj)
{
	if (primitives_count.find(C_CYLINDER)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompCylinder(game_obj, vao_cylinder, shaderPrimitive);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreateCapsule(RE_GameObject* game_obj)
{
	if (primitives_count.find(C_CAPSULE)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompCapsule(game_obj, vao_capsule, shaderPrimitive);
	return ret;
}

unsigned int RE_PrimitiveManager::CheckCubeVAO()
{
	if (vao_cube == 0 && primitives_count.find(C_CUBE)->second++ == 0) {
		par_shapes_mesh* cube = par_shapes_create_cube();

		glGenVertexArrays(1, &vao_cube);
		glGenBuffers(1, &vbo_cube);

		RE_GLCache::ChangeVAO(vao_cube);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_cube);
		glBufferData(GL_ARRAY_BUFFER, cube->npoints * sizeof(float) * 3, cube->points, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &ebo_cube);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_cube);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube->ntriangles * sizeof(unsigned short) * 3, cube->triangles, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		par_shapes_free_mesh(cube);
	}
	return vao_cube;
}

void RE_PrimitiveManager::Rest(unsigned short int count)
{
	primitives_count.find(count)->second--;
	if (primitives_count.find(count)->second == 0)
		DeleteVAOPrimitive(count);
}

bool RE_PrimitiveManager::Init(const char* def_shader)
{
	
	shaderPrimitive = ((RE_Shader*)App->resources->At(App->internalResources->GetDefaultShader()))->GetID();

	App->ReportSoftware("par_shapes.h", nullptr, "https://github.com/prideout/par");

	return true;
}

void RE_PrimitiveManager::DeleteVAOPrimitive(unsigned short int primitive)
{
	switch (primitive)
	{
	case C_AXIS:
		glDeleteVertexArrays(1, &(GLuint)vao_axis);
		glDeleteBuffers(1, &(GLuint)vbo_axis);
		break;

	case C_POINT:
		glDeleteVertexArrays(1, &(GLuint)vao_point);
		glDeleteBuffers(1, &(GLuint)vbo_point);
		break;

	case C_LINE:
		glDeleteVertexArrays(1, &(GLuint)vao_line);
		glDeleteBuffers(1, &(GLuint)vbo_line);
		break;

	case C_RAY:
		glDeleteVertexArrays(1, &(GLuint)vao_ray);
		glDeleteBuffers(1, &(GLuint)vbo_ray);
		break;

	case C_TRIANGLE:
		glDeleteVertexArrays(1, &(GLuint)vao_triangle);
		glDeleteBuffers(1, &(GLuint)vbo_triangle);
		glGenBuffers(1, &(GLuint)ebo_triangle);
		break;

	case C_PLANE:
		glDeleteVertexArrays(1, &(GLuint)vao_plane);
		glDeleteBuffers(1, &(GLuint)vbo_plane);
		glDeleteBuffers(1, &(GLuint)ebo_plane);
		break;

	case C_CUBE:
		glDeleteVertexArrays(1, &(GLuint)vao_cube);
		glDeleteBuffers(1, &(GLuint)vbo_cube);
		glDeleteBuffers(1, &(GLuint)ebo_cube);
		break;

	case C_FUSTRUM:
		glDeleteVertexArrays(1, &(GLuint)vao_fustrum);
		glDeleteBuffers(1, &(GLuint)vbo_fustrum);
		glDeleteBuffers(1, &(GLuint)ebo_fustrum);
		break;

	case C_CYLINDER:
		glDeleteVertexArrays(1, &(GLuint)vao_cylinder);
		glDeleteBuffers(1, &(GLuint)vbo_cylinder);
		glDeleteBuffers(1, &(GLuint)ebo_cylinder);
		break;

	case C_CAPSULE:
		glDeleteVertexArrays(1, &(GLuint)vao_capsule);
		glDeleteBuffers(1, &(GLuint)vbo_capsule);
		glDeleteBuffers(1, &(GLuint)ebo_capsule);
		break;
	}
}
