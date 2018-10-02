#ifndef __RE_COMPPRIMITIVE_H__
#define __RE_COMPPRIMITIVE_H__

#include "RE_CompMesh.h"
#include "MathGeoLib\include\MathGeoLib.h"

enum PrimitiveType : short unsigned int
{
	POINT = 0x00,
	USES_THICK_POINTS,
	LINE = 0x10,
	RAY,
	AXIS, //x xyz
	USES_THICK_LINES,
	TRIANGLE = 0x20,
	SQUARE,
	CUBE,
	SPHERE,
	CYLINDER,
	PLANE,
	CAPSULE,
	FRUSTUM
};

class RE_CompPrimitives : public RE_CompMesh
{
public:
	RE_CompPrimitives();
	RE_CompPrimitives(PrimitiveType t);

	PrimitiveType GetType() const;

protected:

	PrimitiveType type;
	math::float4x4 transform;

	//glLineWidth(2.0f);
	//glPointSize(5.0f);
	
};

typedef RE_CompPrimitives RE_CompPoint;

/**************************************************
******	Line
**************************************************/

class RE_CompLine : public RE_CompPoint
{
public:
	RE_CompLine(math::vec origin = math::vec::zero, math::vec dest = math::vec(0.0f, 5.0f, 0.0f));
	math::vec origin;
	math::vec dest;
};

/**************************************************
******	Ray
**************************************************/

class RE_CompRay : public RE_CompPoint
{
public:
	RE_CompRay(math::vec origin = math::vec::zero, math::vec dir = math::vec(0.0f, 1.0f, 0.0f));
	math::vec origin;
	math::vec dir;
};

/**************************************************
******	Triangle
**************************************************/

class RE_CompTriangle : public RE_CompPoint
{
public:
	RE_CompTriangle();
	RE_CompTriangle(math::vec origin, unsigned int );
};

/**************************************************
******	Square
**************************************************/

class RE_CompSquare : public RE_CompPoint
{
public:
	RE_CompSquare();
	RE_CompSquare(math::vec origin, math::vec end);
};

/**************************************************
******	Cube
**************************************************/

class RE_CompCube : public RE_CompPoint
{
public:
	RE_CompCube();
	RE_CompCube(math::vec origin, math::vec end);
};

/**************************************************
******	Sphere
**************************************************/

class RE_CompSphere : public RE_CompPoint
{
public:
	RE_CompSphere();
	RE_CompSphere(math::vec origin, math::vec end);
};

/**************************************************
******	Cylinder
**************************************************/

class RE_CompCylinder : public RE_CompPoint
{
public:
	RE_CompCylinder();
	RE_CompCylinder(math::vec origin, math::vec end);
};

/**************************************************
******	Axis
**************************************************/

class RE_CompAxis : public RE_CompPoint
{
public:
	RE_CompAxis();
	RE_CompAxis(math::vec origin, math::vec end);
};

/**************************************************
******	Plane
**************************************************/

class RE_CompPlane : public RE_CompPoint
{
public:
	RE_CompPlane();
	RE_CompPlane(math::vec origin, math::vec end);
};

/**************************************************
******	Capsule
**************************************************/

class RE_CompCapsule : public RE_CompPoint
{
public:
	RE_CompCapsule();
	RE_CompCapsule(math::vec origin, math::vec end);
};

/**************************************************
******	Fustrum
**************************************************/

class RE_CompFustrum : public RE_CompPoint
{
public:
	RE_CompFustrum();
	RE_CompFustrum(math::vec origin, math::vec end);
};


#endif // !__RE_COMPPRIMITIVE_H__