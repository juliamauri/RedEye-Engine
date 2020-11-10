#ifndef __RE_PRIMITVEMANAGER_H__
#define __RE_PRIMITVEMANAGER_H__

#include "MathGeoLib/include/Math/float3.h"

class RE_GameObject;
class RE_CompPrimitive;

class RE_PrimitiveManager
{
public:
	RE_PrimitiveManager();
	~RE_PrimitiveManager();

	void Init();

	void SetUpComponentPrimitive(RE_CompPrimitive* cmpP);
	eastl::pair<unsigned int, unsigned int> GetPlatonicData(unsigned short type);

private:

	void UploadPlatonic(struct par_shapes_mesh_s* param, unsigned int* vao, unsigned int* vbo, unsigned int * ebo, unsigned int* triangles);

private:

	struct PlatonicData
	{
		unsigned int vao;
		unsigned int vbo;
		unsigned int ebo;
		unsigned int triangles;
	} platonics[5];
};

#endif // !__RE_PRIMITVEMANAGER_H__#
