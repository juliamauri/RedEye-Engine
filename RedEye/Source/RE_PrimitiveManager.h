#ifndef __RE_PRIMITVEMANAGER_H__
#define __RE_PRIMITVEMANAGER_H__

#include <EASTL/map.h>

#include "MathGeoLib/include/Math/float3.h"

class RE_GameObject;
class RE_CompPrimitive;


class RE_PrimitiveManager
{
public:
	RE_PrimitiveManager();
	~RE_PrimitiveManager();

	bool Init(const char* def_shader);

	 //Create
	RE_CompPrimitive* CreateGrid(RE_GameObject* game_obj);
	RE_CompPrimitive* CreateRock(RE_GameObject* game_obj, int seed = 44891516, int nsubdivisions = 1);
	RE_CompPrimitive* CreateCube(RE_GameObject* game_obj);
	RE_CompPrimitive* CreateDodecahedron(RE_GameObject* game_obj);
	RE_CompPrimitive* CreateTetrahedron(RE_GameObject* game_obj);
	RE_CompPrimitive* CreateOctohedron(RE_GameObject* game_obj);
	RE_CompPrimitive* CreateIcosahedron(RE_GameObject* game_obj);
	RE_CompPrimitive* CreatePlane(RE_GameObject* game_obj, int slices = 3, int stacks = 3);
	RE_CompPrimitive* CreateSphere(RE_GameObject* game_obj, int slices  = 16, int stacks = 18);
	RE_CompPrimitive* CreateCylinder(RE_GameObject* game_obj, int slices = 30, int stacks = 3);
	RE_CompPrimitive* CreateHemiSphere(RE_GameObject* game_obj, int slices = 10, int stacks = 10);
	RE_CompPrimitive* CreateTorus(RE_GameObject* game_obj, int slices = 30, int stacks = 40, float radius = 0.1f);
	RE_CompPrimitive* CreateTrefoilKnot(RE_GameObject* game_obj, int slices = 30, int stacks = 40, float radius = 0.5f);

	//Check Platonics
	unsigned int CheckCubeVAO();
	unsigned int CheckDodecahedronVAO();
	unsigned int CheckTetrahedronVAO();
	unsigned int CheckOctohedronVAO();
	unsigned int CheckIcosahedronVAO();

private:
	void UploadPlatonic(struct par_shapes_mesh_s* param, unsigned int* vao, unsigned int* vbo, unsigned int * ebo, int* triangles);

private:
	//Others
	unsigned int vao_grid = 0, vbo_grid = 0;

	//Platonic Solids
	unsigned int vao_cube = 0, vbo_cube = 0, ebo_cube = 0;
	unsigned int vao_dodo = 0, vbo_dodo = 0, ebo_dodo = 0;
	unsigned int vao_tetra = 0, vbo_tetra = 0, ebo_tetra = 0;
	unsigned int vao_octo = 0, vbo_octo = 0, ebo_octo = 0;
	unsigned int vao_icosa = 0, vbo_icosa = 0, ebo_icosa = 0;
	int cube_triangles = 0, dodo_triangles = 0, tetra_triangles = 0, octo_triangles = 0, icosa_triangles = 0;

	//Shader for primitives
	unsigned int shaderPrimitive = 0;
};

#endif // !__RE_PRIMITVEMANAGER_H__#
