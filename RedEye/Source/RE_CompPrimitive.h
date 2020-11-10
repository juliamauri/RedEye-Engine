#ifndef __RE_COMPPRIMITIVE_H__
#define __RE_COMPPRIMITIVE_H__

#include "RE_Math.h"
#include "RE_Component.h"

#include <EASTL/string.h>

class RE_CompTransform;

class RE_CompPrimitive : public RE_Component
{
public:

	RE_CompPrimitive(ComponentType t);
	virtual ~RE_CompPrimitive() {}

	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const UID parent) override {}

	void Draw() const override {}
	void DrawProperties() override {}

	virtual bool CheckFaceCollision(const math::Ray& ray, float& distance) const;

	void DeleteBuffers();

	void SetColor(float r, float g, float b);
	void SetColor(math::vec nColor);

	unsigned int GetVAO() const;
	virtual void SetVAO(unsigned int vao);

	virtual unsigned int GetTriangleCount() const;

protected:

	unsigned int VAO = 0;
	unsigned int VBO = 0;
	unsigned int EBO = 0;
	unsigned int triangle_count = 0;
	math::vec color = math::vec::one;
};

/**************************************************
******	Grid
**************************************************/

class RE_CompGrid : public RE_CompPrimitive
{
public:

	RE_CompGrid() : RE_CompPrimitive(C_GRID) {}
	~RE_CompGrid();

	void GridSetUp(int divisions = 10);
	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const UID parent) override;

	void Draw() const override;
	void DrawProperties() override;

	RE_CompTransform* GetTransformPtr() const;

	unsigned int GetBinarySize() const override;
	void SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources) const override;
	void DeserializeJson(JSONNode* node, eastl::map<int, const char*>* resources) override;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const override;
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources) override;

private:

	int divisions = 0;
	RE_CompTransform* transform = nullptr;
};

/**************************************************
******	Rock
**************************************************/

class RE_CompRock : public RE_CompPrimitive
{
public:

	RE_CompRock() : RE_CompPrimitive(C_ROCK) {}
	~RE_CompRock();

	void RockSetUp(int _seed = 251654, int _subdivions = 5);
	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const UID parent) override;

	void Draw() const override;
	void DrawProperties() override;

	unsigned int GetBinarySize() const override;
	void SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources) const override;
	void DeserializeJson(JSONNode* node, eastl::map<int, const char*>* resources) override;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const override;
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources) override;

private:

	void GenerateNewRock(int seed, int subdivisions);

private:

	int seed = 251654, nsubdivisions = 5;
	bool canChange = true;
};

/**************************************************
******	Platonic
**************************************************/

class RE_CompPlatonic : public RE_CompPrimitive
{
public:

	RE_CompPlatonic(ComponentType t) : RE_CompPrimitive(t) {}
	~RE_CompPlatonic();

	void PlatonicSetUp(unsigned int vao = 0, unsigned int t_count = 0);
	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const UID parent) override;

	void Draw() const override;
	void DrawProperties() override;

	unsigned int GetBinarySize() const override;
	void SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources) const override;
	void DeserializeJson(JSONNode* node, eastl::map<int, const char*>* resources) override;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const override;
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources) override;

protected:

	eastl::string pName;
};

/**************************************************
******	Parametric
**************************************************/

class RE_CompParametric : public RE_CompPrimitive
{
public:
	RE_CompParametric(ComponentType t, const char* name);
	virtual ~RE_CompParametric();

	void ParametricSetUp(int _slices, int _stacks, float _radius = 0.0f);
	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const UID parent) override;

	void Draw() const override;
	void DrawProperties() override;

	unsigned int GetBinarySize() const override;
	void SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources) const override;
	void DeserializeJson(JSONNode* node, eastl::map<int, const char*>* resources) override;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const override;
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources) override;

protected:

	virtual void GenerateParametric() = 0;
	void UploadParametric(struct par_shapes_mesh_s* param);

protected:

	eastl::string name;

	bool show_checkers = false;
	bool canChange = false;

	int slices = 0, stacks = 0;
	float radius = 0.f, min_r = 0.f, max_r = 0.f;

private:

	int target_slices, target_stacks;
	float target_radius;
};

/**************************************************
******	Platonic
**************************************************/

class RE_CompCube : public RE_CompPlatonic
{ public: RE_CompCube() : RE_CompPlatonic(C_CUBE) {} };

class RE_CompDodecahedron : public RE_CompPlatonic
{ public: RE_CompDodecahedron() : RE_CompPlatonic(C_DODECAHEDRON) {} };

class RE_CompTetrahedron : public RE_CompPlatonic
{ public: RE_CompTetrahedron() : RE_CompPlatonic(C_TETRAHEDRON) {} };

class RE_CompOctohedron : public RE_CompPlatonic
{ public: RE_CompOctohedron() : RE_CompPlatonic(C_OCTOHEDRON) {} };

class RE_CompIcosahedron : public RE_CompPlatonic
{ public: RE_CompIcosahedron() : RE_CompPlatonic(C_ICOSAHEDRON) {} };

/**************************************************
******	Plane - Parametric
**************************************************/

class RE_CompPlane : public RE_CompParametric
{
public:
	RE_CompPlane();
	~RE_CompPlane();

	const char* TransformAsMeshResource();

private:
	void GenerateParametric()override;
};

/**************************************************
******	Sphere - Parametric
**************************************************/

class RE_CompSphere : public RE_CompParametric
{
public:
	RE_CompSphere();
	~RE_CompSphere();

private:
	void GenerateParametric()override;

};

/**************************************************
******	Cylinder - Parametric
**************************************************/

class RE_CompCylinder : public RE_CompParametric
{
public:
	RE_CompCylinder();
	~RE_CompCylinder();

private:
	void GenerateParametric()override;
};

/**************************************************
******	HemiSphere - Parametric
**************************************************/

class RE_CompHemiSphere : public RE_CompParametric
{
public:
	RE_CompHemiSphere();
	~RE_CompHemiSphere();

private:
	void GenerateParametric()override;
};

/**************************************************
******	Torus - Parametric
**************************************************/

class RE_CompTorus : public RE_CompParametric
{
public:
	RE_CompTorus();
	~RE_CompTorus();

private:
	void GenerateParametric()override;

};

/**************************************************
******	Trefoi Knot - Parametric
**************************************************/

class RE_CompTrefoiKnot : public RE_CompParametric
{
public:
	RE_CompTrefoiKnot();
	~RE_CompTrefoiKnot();

private:
	void GenerateParametric()override;
};

#endif // !__RE_COMPPRIMITIVE_H__