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

	void SetUpComponentPrimitive(RE_CompPrimitive* cmpP, RE_GameObject* parent);

	 //Create
	RE_CompPrimitive* CreateGrid(RE_GameObject* game_obj);

	//Check Platonics
	unsigned int CheckPlatonicVAO(unsigned short type);


	//Shader for primitives
	static unsigned int shaderPrimitive;

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
};

#endif // !__RE_PRIMITVEMANAGER_H__#
