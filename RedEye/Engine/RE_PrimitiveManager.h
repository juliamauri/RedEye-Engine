#ifndef __RE_PRIMITVEMANAGER_H__
#define __RE_PRIMITVEMANAGER_H__

#include "RE_Component.h"

class RE_GameObject;
class RE_CompPrimitive;
class RE_CompGrid;
class RE_CompRock;

struct PrimData
{
	uint vao = 0;
	uint vbo = 0;
	uint ebo = 0;
	uint triangles = 0;
	uint refCount = 0;

	PrimData(uint vao = 0, uint vbo = 0, uint ebo = 0, uint triangles = 0, uint refCount = 0)
		: vao(vao), vbo(vbo), ebo(ebo), triangles(triangles), refCount(refCount)
	{}
};

class RE_PrimitiveManager
{
public:
	RE_PrimitiveManager();
	~RE_PrimitiveManager();

	void Init();
	void Clear();

	void SetUpComponentPrimitive(RE_CompPrimitive* cmpP);
	eastl::pair<unsigned int, unsigned int> GetPrimitiveMeshData(RE_Component* primComp, int id = -1);
	void UnUsePrimitive(RE_Component::Type pType, int id = -1);

	void CreateSphere(int slices, int stacks, unsigned int& vao, unsigned int& vbo, unsigned int& ebo, unsigned int& triangles);

private:

	void GeneratePoint(PrimData& prim);
	void GenerateGrid(PrimData& prim, RE_CompGrid* gridC);
	void GenerateRock(PrimData& prim, RE_CompRock* rockC);

	void UploadPlatonic(PrimData& prim, struct par_shapes_mesh_s* param);
	void UploadParametric(PrimData& prim, struct par_shapes_mesh_s* param);

private:

	PrimData *platonics = nullptr;

	eastl::map<RE_Component::Type, eastl::map<int,PrimData>> primReference;
};

#endif // !__RE_PRIMITVEMANAGER_H__#
