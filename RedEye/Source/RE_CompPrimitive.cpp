#include "RE_CompPrimitive.h"

#include "RE_Mesh.h"

#define SMALL_INFINITY 2000

RE_CompPrimitive::RE_CompPrimitive(RE_PrimitiveType t, unsigned int VAO) : VAO(VAO), type(t) {}

RE_PrimitiveType RE_CompPrimitive::GetType() const
{
	return type;
}

RE_CompAxis::RE_CompAxis(unsigned int VAO) : RE_CompPrimitive(RE_AXIS, VAO) {}

void RE_CompAxis::Draw()
{
}

RE_CompPoint::RE_CompPoint(unsigned int VAO, math::vec point) : point(point), RE_CompPrimitive(RE_POINT, VAO) {}

void RE_CompPoint::Draw()
{
}

RE_CompLine::RE_CompLine(unsigned int VAO, math::vec origin, math::vec dest) : origin(origin), dest(dest), RE_CompPrimitive(RE_LINE, VAO) {}

void RE_CompLine::Draw()
{
}

RE_CompRay::RE_CompRay(unsigned int VAO, math::vec origin, math::vec dir) : RE_CompLine(NULL, origin, dir), RE_CompPrimitive(RE_RAY, VAO) {}

void RE_CompRay::Draw()
{
}

RE_CompTriangle::RE_CompTriangle(unsigned int VAO) : RE_CompPrimitive(RE_TRIANGLE, VAO) {}

void RE_CompTriangle::Draw()
{
}

RE_CompPlane::RE_CompPlane(unsigned int VAO) : RE_CompPrimitive(RE_PLANE, VAO) {}

void RE_CompPlane::Draw()
{
}

RE_CompCube::RE_CompCube(unsigned int VAO) : RE_CompPrimitive(RE_CUBE, VAO) {}

void RE_CompCube::Draw()
{
}

RE_CompFustrum::RE_CompFustrum(unsigned int VAO) : RE_CompPrimitive(RE_FUSTRUM, VAO) {}

void RE_CompFustrum::Draw()
{
}

RE_CompSphere::RE_CompSphere(unsigned int VAO) : RE_CompPrimitive(RE_SPHERE, VAO) {}

void RE_CompSphere::Draw()
{
}

RE_CompCylinder::RE_CompCylinder(unsigned int VAO) : RE_CompPrimitive(RE_CYLINDER, VAO) {}

void RE_CompCylinder::Draw()
{
}

RE_CompCapsule::RE_CompCapsule(unsigned int VAO) : RE_CompPrimitive(RE_CAPSULE, VAO) {}

void RE_CompCapsule::Draw()
{
}
