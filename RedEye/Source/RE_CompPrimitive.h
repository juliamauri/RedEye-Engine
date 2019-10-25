#ifndef __RE_COMPPRIMITIVE_H__
#define __RE_COMPPRIMITIVE_H__

#include "RE_Math.h"
#include "RE_Component.h"


class RE_CompPrimitive : public RE_Component
{
public:
	RE_CompPrimitive(ComponentType t, RE_GameObject* game_obj, unsigned int VAO, unsigned int shader = 0);
	virtual ~RE_CompPrimitive();
	
	ComponentType GetType() const;
	virtual void Draw() override = 0;

protected:
	ComponentType type;
	unsigned int VAO;
	unsigned int shader;
};

/**************************************************
******	Axis
**************************************************/

class RE_CompAxis : public RE_CompPrimitive
{
public:
	RE_CompAxis(RE_GameObject* game_obj = nullptr, unsigned int VAO = 0);
	~RE_CompAxis();
	void Draw() override;

private:
	math::float4x4 basis;
};

/**************************************************
******	Point
**************************************************/

class RE_CompPoint : public RE_CompPrimitive, RE_CompAxis
{
public:
	RE_CompPoint(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader, math::vec point = math::vec::zero);
	~RE_CompPoint();
	void Draw() override;

private:
	math::vec point;
};

/**************************************************
******	Line
**************************************************/

class RE_CompLine : public RE_CompPrimitive, RE_CompAxis
{
public:
	RE_CompLine(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader, math::vec origin = math::vec::zero, math::vec dest = math::vec(0.0f, 5.0f, 0.0f));
	~RE_CompLine();
	void Draw() override;

private:
	math::vec origin;
	math::vec dest;
};

/**************************************************
******	Ray
**************************************************/

class RE_CompRay : public RE_CompPrimitive, RE_CompLine, RE_CompAxis
{
public:
	RE_CompRay(RE_GameObject* game_obj, unsigned int shader, unsigned int VAO = 0, math::vec origin = math::vec::zero, math::vec dir = math::vec(0.0f, 1.0f, 0.0f));
	~RE_CompRay();
	void Draw() override;

};

/**************************************************
******	Triangle
**************************************************/

class RE_CompTriangle : public RE_CompPrimitive, RE_CompAxis
{
public:
	RE_CompTriangle(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader);
	~RE_CompTriangle();
	void Draw() override;
};

/**************************************************
******	Plane
**************************************************/

class RE_CompPlane : public RE_CompPrimitive, RE_CompAxis
{
public:
	RE_CompPlane(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader);
	~RE_CompPlane();
	void Draw() override;
};

/**************************************************
******	Cube
**************************************************/

class RE_CompCube : public RE_CompPrimitive, RE_CompAxis
{
public:
	RE_CompCube(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader);
	~RE_CompCube();
	void Draw() override;
};

/**************************************************
******	Fustrum
**************************************************/

class RE_CompFustrum : public RE_CompPrimitive, RE_CompAxis
{
public:
	RE_CompFustrum(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader);
	~RE_CompFustrum();
	void Draw() override;
};

/**************************************************
******	Sphere
**************************************************/

class RE_CompSphere : public RE_CompPrimitive, RE_CompAxis
{
public:
	RE_CompSphere(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader, int triangle_count);
	~RE_CompSphere();
	void Draw() override;
	void DrawProperties() override;

private:
	bool show_checkers = false;
	math::vec color;
	int triangle_count;
};

/**************************************************
******	Cylinder
**************************************************/

class RE_CompCylinder : public RE_CompPrimitive, RE_CompAxis
{
public:
	RE_CompCylinder(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader);
	~RE_CompCylinder();
	void Draw() override;
};


/**************************************************
******	Capsule
**************************************************/

class RE_CompCapsule : public RE_CompPrimitive, RE_CompAxis
{
public:
	RE_CompCapsule(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader);
	~RE_CompCapsule();
	void Draw() override;
};

#endif // !__RE_COMPPRIMITIVE_H__