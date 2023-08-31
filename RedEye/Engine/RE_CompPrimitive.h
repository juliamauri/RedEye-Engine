#ifndef __RE_COMPPRIMITIVE_H__
#define __RE_COMPPRIMITIVE_H__

#include "RE_Component.h"

class RE_CompTransform;

class RE_CompPrimitive : public RE_Component
{
public:

	RE_CompPrimitive(RE_Component::Type t);
	virtual ~RE_CompPrimitive() = default;

	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent) override {}

	void Draw() const override {}
	void SimpleDraw() const;
	void DrawProperties() override {}
	virtual bool DrawPrimPropierties() { return false; }

	virtual bool CheckFaceCollision(const math::Ray& ray, float& distance) const;

	void SetColor(float r, float g, float b);
	void SetColor(math::vec nColor);

	unsigned int GetVAO() const;
	size_t GetTriangleCount() const;

	void UnUseResources();

	virtual size_t GetParticleBinarySize() const { return 0; }
	virtual void SerializeParticleJson(RE_Json* node) const;
	virtual void DeserializeParticleJson(RE_Json* node);
	virtual void SerializeParticleBinary(char*& cursor) const {}
	virtual void DeserializeParticleBinary(char*& cursor) {}


protected:

	int primID = -1;
	unsigned int VAO = 0;
	size_t triangle_count = 0;
	math::vec color = math::vec::one;
};

/**************************************************
******	Grid
**************************************************/

class RE_CompGrid : public RE_CompPrimitive
{
public:

	RE_CompGrid() : RE_CompPrimitive(RE_Component::Type::GRID) {}
	~RE_CompGrid() final = default;
	friend class RE_PrimitiveManager;

	void GridSetUp(int divisions = 10);
	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent) override final;

	void Draw() const override final;
	void DrawProperties() override final;

	RE_CompTransform* GetTransformPtr() const;

	size_t GetBinarySize() const override final;
	void SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources) const override final;
	void DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources) override final;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const override final;
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources) override final;

	float GetDistance() const;

private:

	int divisions = 50, tmpSb = 50;
	float distance = 125.f;
	RE_CompTransform* transform = nullptr;
};

/**************************************************
******	Rock
**************************************************/

class RE_CompRock : public RE_CompPrimitive
{
public:

	RE_CompRock() : RE_CompPrimitive(RE_Component::Type::ROCK) {}
	~RE_CompRock() final = default;
	friend class RE_PrimitiveManager;

	void RockSetUp(int _seed = 251654, int _subdivions = 5);
	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent) override final;

	void Draw() const override final;
	void DrawProperties() override final;
	bool DrawPrimPropierties() override final;

	size_t GetBinarySize() const override final;
	void SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources) const override final;
	void DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources) override final;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const override final;
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources) override final;

	size_t GetParticleBinarySize() const override final;
	void SerializeParticleJson(RE_Json* node) const override final;
	void DeserializeParticleJson(RE_Json* node) override final;
	void SerializeParticleBinary(char*& cursor) const override final;
	void DeserializeParticleBinary(char*& cursor) override final;

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

	RE_CompPlatonic(RE_Component::Type t) : RE_CompPrimitive(t) {}
	~RE_CompPlatonic() = default;
	friend class RE_PrimitiveManager;

	void PlatonicSetUp();
	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent) override final;

	void Draw() const override final;
	void DrawProperties() override final;
	bool DrawPrimPropierties() override final;

	size_t GetBinarySize() const override final;
	void SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources) const override final;
	void DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources) override final;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const override final;
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources) override final;

protected:

	eastl::string pName;
};

/**************************************************
******	Parametric
**************************************************/

class RE_CompParametric : public RE_CompPrimitive
{
public:
	RE_CompParametric(RE_Component::Type t, const char* name);
	virtual ~RE_CompParametric();
	friend class RE_PrimitiveManager;

	void ParametricSetUp(int _slices, int _stacks, float _radius = 0.0f);
	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent) override final;

	void Draw() const override final;
	void DrawProperties() override final;
	bool DrawPrimPropierties() override final;

	size_t GetBinarySize() const override final;
	void SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources) const override final;
	void DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources) override final;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const override final;
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources) override final;

	size_t GetParticleBinarySize() const override final;
	void SerializeParticleJson(RE_Json* node) const override final;
	void DeserializeParticleJson(RE_Json* node) override final;
	void SerializeParticleBinary(char*& cursor) const override final;
	void DeserializeParticleBinary(char*& cursor) override final;

protected:

	eastl::string name;

	bool show_checkers = false;
	bool canChange = false;

	int slices = 0, stacks = 0;
	float radius = 0.f, min_r = 0.f, max_r = 0.f;

private:

	int target_slices = 0, target_stacks = 0;
	float target_radius = 0.f;
};

/**************************************************
******	Platonic
**************************************************/

class RE_CompPoint : public RE_CompPlatonic
{
public:
	RE_CompPoint() : RE_CompPlatonic(RE_Component::Type::POINT) {}
	~RE_CompPoint() final = default;
};

class RE_CompCube : public RE_CompPlatonic
{
public:
	RE_CompCube() : RE_CompPlatonic(RE_Component::Type::CUBE) {}
	~RE_CompCube() final = default;
};

class RE_CompDodecahedron : public RE_CompPlatonic
{
public:
	RE_CompDodecahedron() : RE_CompPlatonic(RE_Component::Type::DODECAHEDRON) {}
	~RE_CompDodecahedron() final = default;
};

class RE_CompTetrahedron : public RE_CompPlatonic
{
public:
	RE_CompTetrahedron() : RE_CompPlatonic(RE_Component::Type::TETRAHEDRON) {}
	~RE_CompTetrahedron() final = default;
};

class RE_CompOctohedron : public RE_CompPlatonic
{
public:
	RE_CompOctohedron() : RE_CompPlatonic(RE_Component::Type::OCTOHEDRON) {}
	~RE_CompOctohedron() final = default;
};

class RE_CompIcosahedron : public RE_CompPlatonic
{
public:
	RE_CompIcosahedron() : RE_CompPlatonic(RE_Component::Type::ICOSAHEDRON) {}
	~RE_CompIcosahedron() final = default;
};

/**************************************************
******	Plane - Parametric
**************************************************/

class RE_CompPlane : public RE_CompParametric
{
public:
	RE_CompPlane::RE_CompPlane() : RE_CompParametric(RE_Component::Type::PLANE, "Plane") {}
	~RE_CompPlane() final = default;

	const char* TransformAsMeshResource();
};

/**************************************************
******	Sphere - Parametric
**************************************************/

class RE_CompSphere : public RE_CompParametric
{
public:
	RE_CompSphere::RE_CompSphere() : RE_CompParametric(RE_Component::Type::SPHERE, "Sphere") {}
	~RE_CompSphere() final = default;
};

/**************************************************
******	Cylinder - Parametric
**************************************************/

class RE_CompCylinder : public RE_CompParametric
{
public:
	RE_CompCylinder::RE_CompCylinder() : RE_CompParametric(RE_Component::Type::CYLINDER, "Cylinder") {}
	~RE_CompCylinder() final = default;
};

/**************************************************
******	HemiSphere - Parametric
**************************************************/

class RE_CompHemiSphere : public RE_CompParametric
{
public:
	RE_CompHemiSphere::RE_CompHemiSphere() : RE_CompParametric(RE_Component::Type::HEMISHPERE, "HemiSphere") {}
	~RE_CompHemiSphere() final = default;
};

/**************************************************
******	Torus - Parametric
**************************************************/

class RE_CompTorus : public RE_CompParametric
{
public:
	RE_CompTorus::RE_CompTorus() : RE_CompParametric(RE_Component::Type::TORUS, "Torus")
	{
		min_r = 0.1f;
		max_r = 1.0f;
	}
	~RE_CompTorus() final = default;
};

/**************************************************
******	Trefoi Knot - Parametric
**************************************************/

class RE_CompTrefoiKnot : public RE_CompParametric
{
public:
	RE_CompTrefoiKnot::RE_CompTrefoiKnot() : RE_CompParametric(RE_Component::Type::TREFOILKNOT, "Trefoil Knot")
	{
		min_r = 0.5f;
		max_r = 3.0f;
	}
	~RE_CompTrefoiKnot() final = default;
};

#endif // !__RE_COMPPRIMITIVE_H__