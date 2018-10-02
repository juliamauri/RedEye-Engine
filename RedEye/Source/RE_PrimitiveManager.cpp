#include "RE_PrimitiveManager.h"

#include "SDL2/include/SDL.h"
#include "Glew/include/glew.h"
#include <gl/GL.h>

#include "Application.h"
#include "ShaderManager.h"

RE_PrimitiveManager::RE_PrimitiveManager()
{
	primitives_count.insert(std::pair<RE_PrimitiveType, unsigned int>(RE_PrimitiveType::RE_POINT, 0));
	primitives_count.insert(std::pair<RE_PrimitiveType, unsigned int>(RE_PrimitiveType::RE_LINE, 0));
	primitives_count.insert(std::pair<RE_PrimitiveType, unsigned int>(RE_PrimitiveType::RE_RAY, 0));
	primitives_count.insert(std::pair<RE_PrimitiveType, unsigned int>(RE_PrimitiveType::RE_AXIS, 0));
	primitives_count.insert(std::pair<RE_PrimitiveType, unsigned int>(RE_PrimitiveType::RE_TRIANGLE, 0));
	primitives_count.insert(std::pair<RE_PrimitiveType, unsigned int>(RE_PrimitiveType::RE_PLANE, 0));
	primitives_count.insert(std::pair<RE_PrimitiveType, unsigned int>(RE_PrimitiveType::RE_CUBE, 0));
	primitives_count.insert(std::pair<RE_PrimitiveType, unsigned int>(RE_PrimitiveType::RE_FUSTRUM, 0));
	primitives_count.insert(std::pair<RE_PrimitiveType, unsigned int>(RE_PrimitiveType::RE_SPHERE, 0));
	primitives_count.insert(std::pair<RE_PrimitiveType, unsigned int>(RE_PrimitiveType::RE_CYLINDER, 0));
	primitives_count.insert(std::pair<RE_PrimitiveType, unsigned int>(RE_PrimitiveType::RE_CAPSULE, 0));
}

RE_PrimitiveManager::~RE_PrimitiveManager()	{}

RE_CompPrimitive * RE_PrimitiveManager::CreateAxis()
{
	if(primitives_count.find(RE_AXIS)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompAxis(vao_axis);
	return ret;
}

RE_CompPrimitive* RE_PrimitiveManager::CreatePoint()
{

	if (primitives_count.find(RE_POINT)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompPoint(vao_axis, shaderPrimitive);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreateLine()
{
	if (primitives_count.find(RE_LINE)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompLine(vao_line, shaderPrimitive);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreateRay()
{
	if (primitives_count.find(RE_RAY)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompRay(vao_ray);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreateTriangle()
{
	if (primitives_count.find(RE_TRIANGLE)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompTriangle(vao_triangle, shaderPrimitive);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreatePlane()
{
	if (primitives_count.find(RE_PLANE)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompPlane(vao_plane, shaderPrimitive);
	return ret;
}

RE_CompCube * RE_PrimitiveManager::CreateCube()
{
	if (primitives_count.find(RE_CUBE)->second++ == 0)
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

	RE_CompCube* ret = new RE_CompCube(vao_cube, shaderPrimitive);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreateFrustum()
{
	if (primitives_count.find(RE_FUSTRUM)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompFustrum(vao_fustrum, shaderPrimitive);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreateSphere()
{
	if (primitives_count.find(RE_SPHERE)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompSphere(vao_sphere, shaderPrimitive);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreateCylinder()
{
	if (primitives_count.find(RE_CYLINDER)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompCylinder(vao_cylinder, shaderPrimitive);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreateCapsule()
{
	if (primitives_count.find(RE_CAPSULE)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompCapsule(vao_capsule, shaderPrimitive);
	return ret;
}

void RE_PrimitiveManager::Rest(RE_PrimitiveType count)
{
	primitives_count.find(count)->second--;
	if (primitives_count.find(count)->second == 0)
		DeleteVAOPrimitive(count);
}

void RE_PrimitiveManager::LoadShader(const char * name)
{
		App->shaders->Load(name, &shaderPrimitive);
}

void RE_PrimitiveManager::DeleteVAOPrimitive(RE_PrimitiveType primitive)
{
	switch (primitive)
	{
	case RE_AXIS:
		glDeleteVertexArrays(1, &vao_axis);
		glDeleteBuffers(1, &vbo_axis);
		break;

	case RE_POINT:
		glDeleteVertexArrays(1, &vao_point);
		glDeleteBuffers(1, &vbo_point);
		break;

	case RE_LINE:
		glDeleteVertexArrays(1, &vao_line);
		glDeleteBuffers(1, &vbo_line);
		break;

	case RE_RAY:
		glDeleteVertexArrays(1, &vao_ray);
		glDeleteBuffers(1, &vbo_ray);
		break;

	case RE_TRIANGLE:
		glDeleteVertexArrays(1, &vao_triangle);
		glDeleteBuffers(1, &vbo_triangle);
		break;

	case RE_PLANE:
		glDeleteVertexArrays(1, &vao_plane);
		glDeleteBuffers(1, &vbo_plane);
		glDeleteBuffers(1, &ebo_plane);				
		break;

	case RE_CUBE:
		glDeleteVertexArrays(1, &vao_cube);
		glDeleteBuffers(1, &vbo_cube);
		glDeleteBuffers(1, &ebo_cube);
		break;

	case RE_FUSTRUM:
		glDeleteVertexArrays(1, &vao_fustrum);
		glDeleteBuffers(1, &vbo_fustrum);
		glDeleteBuffers(1, &ebo_fustrum);
		break;

	case RE_SPHERE:
		glDeleteVertexArrays(1, &vao_sphere);
		glDeleteBuffers(1, &vbo_sphere);
		glDeleteBuffers(1, &ebo_sphere);
		break;

	case RE_CYLINDER:
		glDeleteVertexArrays(1, &vao_cylinder);
		glDeleteBuffers(1, &vbo_cylinder);
		glDeleteBuffers(1, &ebo_cylinder);
		break;

	case RE_CAPSULE:
		glDeleteVertexArrays(1, &vao_capsule);
		glDeleteBuffers(1, &vbo_capsule);
		glDeleteBuffers(1, &ebo_capsule);
		break;
	}
}
