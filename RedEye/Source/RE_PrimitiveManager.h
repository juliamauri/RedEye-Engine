#ifndef __RE_PRIMITVEMANAGER_H__
#define __RE_PRIMITVEMANAGER_H__

#include "RE_CompPrimitive.h"

#include <map>

class RE_PrimitiveManager
{
public:
	RE_PrimitiveManager();

	RE_CompPrimitive* CreateAxis();
	RE_CompPrimitive* CreatePoint();
	RE_CompPrimitive* CreateLine();
	RE_CompPrimitive* CreateRay();
	RE_CompPrimitive* CreateTriangle();
	RE_CompPrimitive* CreatePlane();
	RE_CompPrimitive* CreateCube();
	RE_CompPrimitive* CreateFrustum();
	RE_CompPrimitive* CreateSphere();
	RE_CompPrimitive* CreateCylinder();
	RE_CompPrimitive* CreateCapsule();

private:
	//Vertex Array Object
	unsigned int vao_point = 0, vao_line = 0, vao_ray = 0, vao_axis = 0, vao_triangle = 0, vao_plane = 0,
		vao_cube = 0, vao_fustrum = 0, vao_sphere = 0, vao_cylinder = 0, vao_capsule = 0;
	
	//Vertex Buffer Object
	unsigned int vbo_point = 0, vbo_line = 0, vbo_ray = 0, vbo_axis = 0, vbo_triangle = 0, vbo_plane = 0,
		vbo_cube = 0, vbo_fustrum = 0, vbo_sphere = 0, vbo_cylinder = 0, vbo_capsule = 0;

	//Element Buffer Objects
	unsigned int ebo_plane = 0, ebo_cube = 0, ebo_fustrum = 0, ebo_sphere = 0, ebo_cylinder = 0, ebo_capsule = 0;

	//Primitives Count
	std::map<RE_PrimitiveType, unsigned int> primitives_count;

	//count
	inline void operator+=(RE_PrimitiveType count);
	inline void operator-=(RE_PrimitiveType count);

	//Delete VAO of primitive when its count is 0
	void DeleteVAOPrimitive(RE_PrimitiveType primitive);
};

#endif // !__RE_PRIMITVEMANAGER_H__#
