#ifndef __RE_COMPPRIMITIVE_H__
#define __RE_COMPPRIMITIVE_H__

#include "RE_Component.h"

class RE_CompTransform;

class RE_CompPrimitive : public RE_Component
{
protected:

	int primID = -1;
	uint VAO = 0;
	size_t triangle_count = 0;
	math::vec color = math::vec::one;

public:

	RE_CompPrimitive(RE_Component::Type t);
	virtual ~RE_CompPrimitive() = default;

	virtual void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent) override {}

	void SimpleDraw() const;
	virtual bool DrawPrimPropierties() { return false; }
	virtual bool CheckFaceCollision(const math::Ray& ray, float& distance) const
	{
		// TODO Rub: Primitive raycast checking
		return false;
	}

	static bool IsPrimitive(RE_Component::Type t)
	{
		return t > RE_Component::Type::PRIMIVE_MIN && t < RE_Component::Type::PRIMIVE_MAX;
	}

	uint GetVAO() const { return VAO; }
	size_t GetTriangleCount() const { return triangle_count; }

	void SetColor(math::vec nColor) { color = nColor; }
	void SetColor(float r, float g, float b) { color.Set(r, g, b); }

	void UnUseResources();

	// Particle Serialization
	virtual void ParticleJsonSerialize(RE_Json* node) const;
	virtual void ParticleJsonDeserialize(RE_Json* node);
	virtual size_t GetParticleBinarySize() const { return 0; }
	virtual void ParticleBinarySerialize(char*& cursor) const {}
	virtual void ParticleBinaryDeserialize(char*& cursor) {}
};

#pragma region Grid **************************************************

class RE_CompGrid : public RE_CompPrimitive
{
private:

	int divisions = 50;
	int tmpSb = 50;
	float distance = 125.f;
	RE_CompTransform* transform = nullptr;

public:

	friend class RE_PrimitiveManager;

	RE_CompGrid() : RE_CompPrimitive(RE_Component::Type::GRID) {}
	~RE_CompGrid() final = default;

	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent) final;
	void GridSetUp(int divisions = 10);

	void Draw() const final;
	void DrawProperties() final;

	float GetDistance() const { return distance; }
	RE_CompTransform* GetTransformPtr() const;

	// Serialization
	void JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources) const final;
	void JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources) final;
	size_t GetBinarySize() const final;
	void BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources) const final;
	void BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources) final;
};

#pragma endregion

#pragma region Rock **************************************************

class RE_CompRock : public RE_CompPrimitive
{
private:

	int seed = 251654;
	int nsubdivisions = 5;
	bool canChange = true;

public:

	friend class RE_PrimitiveManager;

	RE_CompRock() : RE_CompPrimitive(RE_Component::Type::ROCK) {}
	~RE_CompRock() final = default;

	void RockSetUp(int _seed = 251654, int _subdivions = 5);
	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent) final;

	void Draw() const final;
	void DrawProperties() final;
	bool DrawPrimPropierties() final;

	// Serialization
	void JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources) const final;
	void JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources) final;
	size_t GetBinarySize() const final;
	void BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources) const final;
	void BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources) final;

	// Particle Serialization
	void ParticleJsonSerialize(RE_Json* node) const final;
	void ParticleJsonDeserialize(RE_Json* node) final;
	size_t GetParticleBinarySize() const final;
	void ParticleBinarySerialize(char*& cursor) const final;
	void ParticleBinaryDeserialize(char*& cursor) final;

private:

	void GenerateNewRock(int seed, int subdivisions);
};

#pragma endregion

#pragma region Platonic **************************************************

class RE_CompPlatonic : public RE_CompPrimitive
{
protected:

	eastl::string pName;

public:

	friend class RE_PrimitiveManager;

	RE_CompPlatonic(RE_Component::Type t) : RE_CompPrimitive(t) {}
	~RE_CompPlatonic() = default;

	void PlatonicSetUp();
	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent) final;

	void Draw() const final;
	void DrawProperties() final;
	bool DrawPrimPropierties() final { return false; }

	// Serialization
	void JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources) const final;
	void JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources) final;
	size_t GetBinarySize() const final;
	void BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources) const final;
	void BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources) final;
};

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

#pragma endregion

#pragma region Parametric **************************************************

class RE_CompParametric : public RE_CompPrimitive
{
protected:

	eastl::string name;

	bool show_checkers = false;
	bool canChange = false;

	int slices = 0;
	int stacks = 0;

	float radius = 0.f;
	float min_r = 0.f;
	float max_r = 0.f;

private:

	int target_slices = 0;
	int target_stacks = 0;
	float target_radius = 0.f;

public:

	friend class RE_PrimitiveManager;

	RE_CompParametric(RE_Component::Type t, const char* _name) : RE_CompPrimitive(t), name(_name) {}
	virtual ~RE_CompParametric() = default;

	void ParametricSetUp(int _slices, int _stacks, float _radius = 0.0f);
	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent) final;

	void Draw() const final;
	void DrawProperties() final;
	bool DrawPrimPropierties() final;

	// Serialization
	void JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources) const final;
	void JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources) final;
	size_t GetBinarySize() const final;
	void BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources) const final;
	void BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources) final;

	// Particle Serialization
	void ParticleJsonSerialize(RE_Json* node) const final;
	void ParticleJsonDeserialize(RE_Json* node) final;
	size_t GetParticleBinarySize() const final;
	void ParticleBinarySerialize(char*& cursor) const final;
	void ParticleBinaryDeserialize(char*& cursor) final;
};

class RE_CompPlane : public RE_CompParametric
{
public:
	RE_CompPlane::RE_CompPlane() : RE_CompParametric(RE_Component::Type::PLANE, "Plane") {}
	~RE_CompPlane() final = default;

	const char* TransformAsMeshResource();
};

class RE_CompSphere : public RE_CompParametric
{
public:
	RE_CompSphere::RE_CompSphere() : RE_CompParametric(RE_Component::Type::SPHERE, "Sphere") {}
	~RE_CompSphere() final = default;
};

class RE_CompCylinder : public RE_CompParametric
{
public:
	RE_CompCylinder::RE_CompCylinder() : RE_CompParametric(RE_Component::Type::CYLINDER, "Cylinder") {}
	~RE_CompCylinder() final = default;
};

class RE_CompHemiSphere : public RE_CompParametric
{
public:
	RE_CompHemiSphere::RE_CompHemiSphere() : RE_CompParametric(RE_Component::Type::HEMISHPERE, "HemiSphere") {}
	~RE_CompHemiSphere() final = default;
};

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

#pragma endregion

#endif // !__RE_COMPPRIMITIVE_H__