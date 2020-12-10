#ifndef __RE_PRIMITVE_MANAGER_H__
#define __RE_PRIMITVE_MANAGER_H__

class RE_CompPrimitive;
struct par_shapes_mesh_s;

namespace RE_PrimitiveManager
{
	void Init();
	void Clear();

	void SetUpComponentPrimitive(RE_CompPrimitive* cmpP);
	void GetPlatonicData(unsigned short type, unsigned int& vao, unsigned int& triangles);

	void CreateSphere(
		unsigned int slices, unsigned int stacks,
		unsigned int& vao , unsigned int& vbo, unsigned int& ebo, unsigned int& triangles);

	namespace Internal
	{
		struct PlatonicData { unsigned int vao = 0, vbo = 0, ebo = 0, triangles = 0; }
		static platonics[5] = {};

		void UploadPlatonic(par_shapes_mesh_s* plato,
			unsigned int& vao, unsigned int& vbo, unsigned int& ebo, unsigned int& triangles);
	};
};

#endif // !__RE_PRIMITVEMANAGER_H__#
