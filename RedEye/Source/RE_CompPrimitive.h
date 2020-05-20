#ifndef __RE_COMPPRIMITIVE_H__
#define __RE_COMPPRIMITIVE_H__

#include "RE_Math.h"
#include "RE_Component.h"

#include <EASTL/string.h>

class RE_CompPrimitive : public RE_Component
{
public:
	RE_CompPrimitive(ComponentType t, RE_GameObject* game_obj, unsigned int VAO = 0, unsigned int shader = 0);
	virtual ~RE_CompPrimitive();
	
	ComponentType GetType() const;
	virtual void Draw() override = 0;
	virtual void DrawProperties() override = 0;
	virtual unsigned int GetBinarySize()const override { return 0; }
	virtual void SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources) override { }
	virtual void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) override { }

	void SetColor(float r, float g, float b) { color.Set(r, g, b); }
	void SetColor(math::vec nColor) { color = nColor; }

	unsigned int GetVAO()const { return VAO; }
	virtual unsigned int GetTriangleCount()const { return 0; }

protected:
	ComponentType type;
	unsigned int VAO = 0;
	unsigned int VBO = 0;
	unsigned int EBO = 0;

	unsigned int shader;

	math::vec color = math::vec::one;
};


/**************************************************
******	Grid
**************************************************/

class RE_CompGrid : public RE_CompPrimitive
{
public:
	RE_CompGrid(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader);
	~RE_CompGrid();
	void Draw() override;
	void DrawProperties() override {}
	unsigned int GetBinarySize()const override { return 0; }
	void SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources) override {}
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) override {}

};

/**************************************************
******	Cube
**************************************************/

class RE_CompCube : public RE_CompPrimitive
{
public:
	RE_CompCube(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader, int triangle_count);
	RE_CompCube(const RE_CompCube& cmpCube, RE_GameObject* go = nullptr);
	~RE_CompCube();
	void Draw() override;
	void DrawProperties() override;
	unsigned int GetBinarySize()const override;
	void SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources) override;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) override;

	unsigned int GetTriangleCount()const override { return triangle_count; }

private:
	bool show_checkers = false;
	int triangle_count;

};

/**************************************************
******	Parametric
**************************************************/

class RE_CompParametric : public RE_CompPrimitive
{
public:
	RE_CompParametric(ComponentType t, const char* name, RE_GameObject* game_obj, unsigned int shader, int _slice, int _stacks, bool _useRadius = false, float _radius = 0.0f, float _minR = 0.0f, float _maxR = 0.0f);
	virtual ~RE_CompParametric();
	void Draw() override;
	void DrawProperties() override;
	unsigned int GetBinarySize()const override;
	void SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources) override;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) override;

	unsigned int GetTriangleCount()const override { return triangle_count; }

protected:
	virtual void GenerateParametric() = 0;
	void UploadParametric(struct par_shapes_mesh_s* param);

protected:
	bool show_checkers = false;
	int triangle_count;
	int slice, stacks;
	bool useRadius = false;
	float radius, minR, maxR;
	bool canChange = true;
	eastl::string pName;
public:
	int tmpSl, tmpSt;
	float tmpR;
};

/**************************************************
******	Plane
**************************************************/

class RE_CompPlane : public RE_CompParametric
{
public:
	RE_CompPlane(RE_GameObject* game_obj, unsigned int shader, int slice, int stacks);
	RE_CompPlane(const RE_CompPlane& cmpPlane, RE_GameObject* go = nullptr);
	~RE_CompPlane();

	const char* TransformAsMeshResource();

private:
	void GenerateParametric()override;
};


/**************************************************
******	Sphere
**************************************************/

class RE_CompSphere : public RE_CompParametric
{
public:
	RE_CompSphere(RE_GameObject* game_obj, unsigned int shader, int slice, int stacks);
	RE_CompSphere(const RE_CompSphere& cmpSphere, RE_GameObject* go = nullptr);
	~RE_CompSphere();

private:
	void GenerateParametric()override;

};

/**************************************************
******	Cylinder
**************************************************/

class RE_CompCylinder : public RE_CompParametric
{
public:
	RE_CompCylinder(RE_GameObject* game_obj, unsigned int shader, int slice, int stacks);
	RE_CompCylinder(const RE_CompCylinder& cmpCylinder, RE_GameObject* go = nullptr);
	~RE_CompCylinder();

private:
	void GenerateParametric()override;
};


/**************************************************
******	HemiSphere
**************************************************/

class RE_CompHemiSphere : public RE_CompParametric
{
public:
	RE_CompHemiSphere(RE_GameObject* game_obj, unsigned int shader, int _slice, int _stacks);
	RE_CompHemiSphere(const RE_CompHemiSphere& cmpHemiSphere, RE_GameObject* go = nullptr);
	~RE_CompHemiSphere();

private:
	void GenerateParametric()override;
};

/**************************************************
******	Torus
**************************************************/

class RE_CompTorus : public RE_CompParametric
{
public:
	RE_CompTorus(RE_GameObject* game_obj, unsigned int shader, int slice, int stacks, float radius);
	RE_CompTorus(const RE_CompTorus& cmpSphere, RE_GameObject* go = nullptr);
	~RE_CompTorus();

private:
	void GenerateParametric()override;

};

/**************************************************
******	Trefoi Knot
**************************************************/

class RE_CompTrefoiKnot : public RE_CompParametric
{
public:
	RE_CompTrefoiKnot(RE_GameObject* game_obj, unsigned int shader, int _slice, int _stacks, float _radius);
	RE_CompTrefoiKnot(const RE_CompTrefoiKnot& cmpTrefoiKnot, RE_GameObject* go = nullptr);
	~RE_CompTrefoiKnot();

private:
	void GenerateParametric()override;
};

/**************************************************
******	Rock
**************************************************/

class RE_CompRock : public RE_CompPrimitive
{
public:
	RE_CompRock(RE_GameObject* game_obj, unsigned int shader, int _seed, int _subdivions);
	RE_CompRock(const RE_CompRock& cmpRock, RE_GameObject* go = nullptr);
	~RE_CompRock();
	void Draw() override;
	void DrawProperties() override;
	unsigned int GetBinarySize()const override;
	void SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources) override;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) override;

	unsigned int GetTriangleCount()const override { return triangle_count; }

private:
	void GenerateNewRock(int seed, int subdivisions);

private:
	bool show_checkers = false;
	int triangle_count;
	int seed, nsubdivisions;
	bool canChange = true;
public:
	int tmpSe, tmpSb;
};

#endif // !__RE_COMPPRIMITIVE_H__