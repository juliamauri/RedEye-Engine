#include "RE_PrimitiveManager.h"

#include "SDL2/include/SDL.h"
#include "Glew/include/glew.h"
#include <gl/GL.h>

#include "Application.h"
#include "ShaderManager.h"

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

RE_PrimitiveManager::~RE_PrimitiveManager()	{}

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
		glGenVertexArrays(1, &vao_point);
		glGenBuffers(1, &vbo_point);

		glBindVertexArray(vao_point);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_point);
		glBufferData(GL_ARRAY_BUFFER, sizeof(pos), &pos, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
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

		glBindVertexArray(vao_line);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_line);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vecline) * sizeof(math::vec), &vecline[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(vecline), (void*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
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

		glBindVertexArray(vao_triangle);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vPositionTriangle) * sizeof(math::vec), &vPositionTriangle[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_triangle);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(vPositionTriangle), (void*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
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

	}
	RE_CompPrimitive* ret = new RE_CompPlane(game_obj, vao_plane, shaderPrimitive);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreateCube(RE_GameObject* game_obj)
{
	if (primitives_count.find(C_CUBE)->second++ == 0)
	{
		//Cube with index
		float vPositionCubeArray[] = {
			//vertecies        
			1.0f,  1.0f, 1.0f,   //Top Right Back - Vert 0
			1.0f, -1.0f, 1.0f,   //Bottom Right Back - Vert 1
			-1.0f, -1.0f, 1.0f,  //Bottom Left Back - Vert 2
			-1.0f,  1.0f, 1.0f,  //Top Left Back - Vert 3
			1.0f,  1.0f, -1.0f,  //Top Right Front - Vert 4
			1.0f, -1.0f, -1.0f,  //Bottom Right Front - Vert 5
			-1.0f, -1.0f, -1.0f,  //Bottom Left Front - Vert 6
			-1.0f,  1.0f, -1.0f   //Top Left Front - Vert 7
		};

		unsigned int indicesCube[] = {  //Tell OpenGL What triangle uses what Vertecies
			0, 1, 3,   //Back Quad
			1, 2, 3,
			0, 1, 4,     //Right Quad
			1, 5, 4,
			2, 3, 7,   //Left Quad
			2, 6, 7,
			4, 5, 7,   //Front Quad
			5, 6, 7,
			0, 3, 4,   //Top Quad
			3, 4, 7,
			1, 2, 5,   //Bottom Quad
			2, 5, 6
		};

		glGenVertexArrays(1, &vao_cube);
		glGenBuffers(1, &vbo_cube);
		glGenBuffers(1, &ebo_cube);

		glBindVertexArray(vao_cube);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_cube);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vPositionCubeArray), vPositionCubeArray, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_cube);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesCube), indicesCube, GL_STATIC_DRAW);
	
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	RE_CompCube* ret = new RE_CompCube(game_obj, vao_cube, shaderPrimitive);
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

RE_CompPrimitive * RE_PrimitiveManager::CreateSphere(RE_GameObject* game_obj)
{
	if (primitives_count.find(C_SPHERE)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompSphere(game_obj, vao_sphere, shaderPrimitive);
	return ret;
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

void RE_PrimitiveManager::Rest(ComponentType count)
{
	primitives_count.find(count)->second--;
	if (primitives_count.find(count)->second == 0)
		DeleteVAOPrimitive(count);
}

void RE_PrimitiveManager::LoadShader(const char * name)
{
		App->shaders->Load(name, &shaderPrimitive);
}

void RE_PrimitiveManager::DeleteVAOPrimitive(ComponentType primitive)
{
	switch (primitive)
	{
	case C_AXIS:
		glDeleteVertexArrays(1, &vao_axis);
		glDeleteBuffers(1, &vbo_axis);
		break;

	case C_POINT:
		glDeleteVertexArrays(1, &vao_point);
		glDeleteBuffers(1, &vbo_point);
		break;

	case C_LINE:
		glDeleteVertexArrays(1, &vao_line);
		glDeleteBuffers(1, &vbo_line);
		break;

	case C_RAY:
		glDeleteVertexArrays(1, &vao_ray);
		glDeleteBuffers(1, &vbo_ray);
		break;

	case C_TRIANGLE:
		glDeleteVertexArrays(1, &vao_triangle);
		glDeleteBuffers(1, &vbo_triangle);
		glGenBuffers(1, &ebo_triangle);
		break;

	case C_PLANE:
		glDeleteVertexArrays(1, &vao_plane);
		glDeleteBuffers(1, &vbo_plane);
		glDeleteBuffers(1, &ebo_plane);				
		break;

	case C_CUBE:
		glDeleteVertexArrays(1, &vao_cube);
		glDeleteBuffers(1, &vbo_cube);
		glDeleteBuffers(1, &ebo_cube);
		break;

	case C_FUSTRUM:
		glDeleteVertexArrays(1, &vao_fustrum);
		glDeleteBuffers(1, &vbo_fustrum);
		glDeleteBuffers(1, &ebo_fustrum);
		break;

	case C_SPHERE:
		glDeleteVertexArrays(1, &vao_sphere);
		glDeleteBuffers(1, &vbo_sphere);
		glDeleteBuffers(1, &ebo_sphere);
		break;

	case C_CYLINDER:
		glDeleteVertexArrays(1, &vao_cylinder);
		glDeleteBuffers(1, &vbo_cylinder);
		glDeleteBuffers(1, &ebo_cylinder);
		break;

	case C_CAPSULE:
		glDeleteVertexArrays(1, &vao_capsule);
		glDeleteBuffers(1, &vbo_capsule);
		glDeleteBuffers(1, &ebo_capsule);
		break;
	}
}
