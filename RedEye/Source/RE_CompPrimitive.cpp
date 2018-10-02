#include "RE_CompPrimitive.h"

#include "SDL2/include/SDL.h"
#include "Glew/include/glew.h"
#include <gl/GL.h>
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "RE_Camera.h"
#include "ShaderManager.h"
#include "RE_PrimitiveManager.h"

#define SMALL_INFINITY 2000

RE_CompPrimitive::RE_CompPrimitive(RE_PrimitiveType t, unsigned int VAO, unsigned int shader) : VAO(VAO), type(t), shader(shader) {}

RE_CompPrimitive::~RE_CompPrimitive()
{
	App->primitives->Rest(type);
}

RE_PrimitiveType RE_CompPrimitive::GetType() const
{
	return type;
}

RE_CompAxis::RE_CompAxis(unsigned int VAO) : RE_CompPrimitive(RE_AXIS, VAO) {}

RE_CompAxis::~RE_CompAxis()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompAxis::Draw()
{
}

RE_CompPoint::RE_CompPoint(unsigned int VAO, unsigned int shader, math::vec point) : point(point), RE_CompPrimitive(RE_POINT, VAO, shader) {}

RE_CompPoint::~RE_CompPoint()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompPoint::Draw()
{
}

RE_CompLine::RE_CompLine(unsigned int VAO, unsigned int shader, math::vec origin, math::vec dest) : origin(origin), dest(dest), RE_CompPrimitive(RE_LINE, VAO, shader) {}

RE_CompLine::~RE_CompLine()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompLine::Draw()
{
}

RE_CompRay::RE_CompRay(unsigned int shader, unsigned int VAO, math::vec origin, math::vec dir) : RE_CompLine(NULL, NULL, origin, dir), RE_CompPrimitive(RE_RAY, VAO, shader) {}

RE_CompRay::~RE_CompRay()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompRay::Draw()
{
}

RE_CompTriangle::RE_CompTriangle(unsigned int VAO, unsigned int shader) : RE_CompPrimitive(RE_TRIANGLE, VAO, shader) {}

RE_CompTriangle::~RE_CompTriangle()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompTriangle::Draw()
{
}

RE_CompPlane::RE_CompPlane(unsigned int VAO, unsigned int shader) : RE_CompPrimitive(RE_PLANE, VAO, shader) {}

RE_CompPlane::~RE_CompPlane()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompPlane::Draw()
{
}

RE_CompCube::RE_CompCube(unsigned int VAO, unsigned int shader) : RE_CompPrimitive(RE_CUBE, VAO, shader) {}

RE_CompCube::~RE_CompCube()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompCube::Draw()
{
	ShaderManager::use(RE_CompPrimitive::shader);
	math::float4x4 model = math::float4x4::Translate(math::float3(0.0f, 3.0f, 0.0f).Neg());
	model.InverseTranspose();
	ShaderManager::setFloat4x4(RE_CompPrimitive::shader, "model", model.ptr());
	ShaderManager::setFloat4x4(RE_CompPrimitive::shader, "view", App->renderer3d->camera->GetView().ptr());
	ShaderManager::setFloat4x4(RE_CompPrimitive::shader, "projection", App->renderer3d->camera->GetProjection().ptr());
	ShaderManager::setFloat(RE_CompPrimitive::shader, "objectColor", math::vec(1.0f, 0.0f, 1.0f));

	glBindVertexArray(RE_CompPrimitive::VAO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

RE_CompFustrum::RE_CompFustrum(unsigned int VAO, unsigned int shader) : RE_CompPrimitive(RE_FUSTRUM, VAO, shader) {}

RE_CompFustrum::~RE_CompFustrum()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompFustrum::Draw()
{
}

RE_CompSphere::RE_CompSphere(unsigned int VAO, unsigned int shader) : RE_CompPrimitive(RE_SPHERE, VAO, shader) {}

RE_CompSphere::~RE_CompSphere()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompSphere::Draw()
{
}

RE_CompCylinder::RE_CompCylinder(unsigned int VAO, unsigned int shader) : RE_CompPrimitive(RE_CYLINDER, VAO, shader) {}

RE_CompCylinder::~RE_CompCylinder()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompCylinder::Draw()
{
}

RE_CompCapsule::RE_CompCapsule(unsigned int VAO, unsigned int shader) : RE_CompPrimitive(RE_CAPSULE, VAO, shader) {}

RE_CompCapsule::~RE_CompCapsule()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompCapsule::Draw()
{
}
