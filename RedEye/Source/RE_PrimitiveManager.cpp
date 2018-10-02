#include "RE_PrimitiveManager.h"

#include "SDL2/include/SDL.h"
#include "Glew/include/glew.h"
#include <gl/GL.h>

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
	RE_CompPrimitive* ret = new RE_CompPoint(vao_axis);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreateLine()
{
	if (primitives_count.find(RE_LINE)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompLine(vao_line);
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
	RE_CompPrimitive* ret = new RE_CompTriangle(vao_triangle);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreatePlane()
{
	if (primitives_count.find(RE_PLANE)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompPlane(vao_plane);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreateCube()
{
	if (primitives_count.find(RE_CUBE)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompCube(vao_cube);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreateFrustum()
{
	if (primitives_count.find(RE_FUSTRUM)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompFustrum(vao_fustrum);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreateSphere()
{
	if (primitives_count.find(RE_SPHERE)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompSphere(vao_sphere);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreateCylinder()
{
	if (primitives_count.find(RE_CYLINDER)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompCylinder(vao_cylinder);
	return ret;
}

RE_CompPrimitive * RE_PrimitiveManager::CreateCapsule()
{
	if (primitives_count.find(RE_CAPSULE)->second++ == 0)
	{

	}
	RE_CompPrimitive* ret = new RE_CompCapsule(vao_capsule);
	return ret;
}

inline void RE_PrimitiveManager::operator+=(RE_PrimitiveType count)
{
	primitives_count.find(count)->second++;
}

inline void RE_PrimitiveManager::operator-=(RE_PrimitiveType count)
{
	primitives_count.find(count)->second--;
	if (primitives_count.find(count)->second == 0)
		DeleteVAOPrimitive(count);
}

void RE_PrimitiveManager::DeleteVAOPrimitive(RE_PrimitiveType primitive)
{
	for (std::map<RE_PrimitiveType, unsigned int>::iterator it = primitives_count.begin(); it != primitives_count.end(); ++it)
		if (it->second > 0)
			switch (it->first)
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
