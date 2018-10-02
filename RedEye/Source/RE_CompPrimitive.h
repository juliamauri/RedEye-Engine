#ifndef __RE_COMPPRIMITIVE_H__
#define __RE_COMPPRIMITIVE_H__

#include "RE_Math.h"

enum RE_PrimitiveType : short unsigned int
{
	RE_AXIS = 0x00, //x xyz
	RE_POINT,
	RE_USES_THICK_POINTS,
	RE_LINE = 0x10,
	RE_RAY,
	RE_USES_THICK_LINES,
	RE_TRIANGLE = 0x20,
	RE_PLANE,
	RE_CUBE,
	RE_FUSTRUM,
	RE_SPHERE,
	RE_CYLINDER,
	RE_CAPSULE
};

class RE_CompPrimitive
{
public:
	RE_CompPrimitive(RE_PrimitiveType t, unsigned int VAO);

	RE_PrimitiveType GetType() const;
	virtual void Draw() {};

protected:
	RE_PrimitiveType type;
	unsigned int VAO;
};

/**************************************************
******	Axis
**************************************************/

class RE_CompAxis : public RE_CompPrimitive
{
public:
	RE_CompAxis(unsigned int VAO = 0);
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
	RE_CompPoint(unsigned int VAO, math::vec point = math::vec::zero);
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
	RE_CompLine(unsigned int VAO, math::vec origin = math::vec::zero, math::vec dest = math::vec(0.0f, 5.0f, 0.0f));
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
	RE_CompRay(unsigned int VAO = 0, math::vec origin = math::vec::zero, math::vec dir = math::vec(0.0f, 1.0f, 0.0f));
	void Draw() override;

};

/**************************************************
******	Triangle
**************************************************/

class RE_CompTriangle : public RE_CompPrimitive, RE_CompAxis
{
public:
	RE_CompTriangle(unsigned int VAO);
	void Draw() override;
};

/**************************************************
******	Plane
**************************************************/

class RE_CompPlane : public RE_CompPrimitive, RE_CompAxis
{
public:
	RE_CompPlane(unsigned int VAO);
	void Draw() override;
};

/**************************************************
******	Cube
**************************************************/

class RE_CompCube : public RE_CompPrimitive, RE_CompAxis
{
public:
	RE_CompCube(unsigned int VAO);
	void Draw() override;
};

/**************************************************
******	Fustrum
**************************************************/

class RE_CompFustrum : public RE_CompPrimitive, RE_CompAxis
{
public:
	RE_CompFustrum(unsigned int VAO);
	void Draw() override;
};

/**************************************************
******	Sphere
**************************************************/

class RE_CompSphere : public RE_CompPrimitive, RE_CompAxis
{
public:
	RE_CompSphere(unsigned int VAO);
	void Draw() override;
};

/**************************************************
******	Cylinder
**************************************************/

class RE_CompCylinder : public RE_CompPrimitive, RE_CompAxis
{
public:
	RE_CompCylinder(unsigned int VAO);
	void Draw() override;
};


/**************************************************
******	Capsule
**************************************************/

class RE_CompCapsule : public RE_CompPrimitive, RE_CompAxis
{
public:
	RE_CompCapsule(unsigned int VAO);
	void Draw() override;
};

#endif // !__RE_COMPPRIMITIVE_H__