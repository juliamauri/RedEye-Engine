#include "RE_CompPrimitive.h"

#include "SDL2/include/SDL.h"
#include "Glew/include/glew.h"
#include <gl/GL.h>
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleEditor.h"
#include "ShaderManager.h"
#include "RE_PrimitiveManager.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_CompCamera.h"
#include "ModuleScene.h"

#define SMALL_INFINITY 2000

RE_CompPrimitive::RE_CompPrimitive(ComponentType t, RE_GameObject* game_obj, unsigned int VAO, unsigned int shader) : VAO(VAO), type(t), shader(shader), RE_Component(t, game_obj) {}

RE_CompPrimitive::~RE_CompPrimitive()
{
	App->primitives->Rest(type);
}

ComponentType RE_CompPrimitive::GetType() const
{
	return type;
}

RE_CompAxis::RE_CompAxis(RE_GameObject* game_obj, unsigned int VAO) : RE_CompPrimitive(C_AXIS, game_obj, VAO) {}

RE_CompAxis::~RE_CompAxis()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompAxis::Draw()
{
}

RE_CompPoint::RE_CompPoint(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader, math::vec point) : point(point), RE_CompPrimitive(C_POINT, game_obj, VAO, shader)  {}

RE_CompPoint::~RE_CompPoint()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompPoint::Draw()
{
	RE_CompPrimitive::RE_Component::go->GetTransform()->SetPosition(point);
	ShaderManager::use(RE_CompPrimitive::shader);
	ShaderManager::setFloat4x4(RE_CompPrimitive::shader, "model", RE_CompPrimitive::RE_Component::go->GetTransform()->GetShaderModel());
	ShaderManager::setFloat4x4(RE_CompPrimitive::shader, "view", App->editor->GetCamera()->GetViewPtr());
	ShaderManager::setFloat4x4(RE_CompPrimitive::shader, "projection", App->editor->GetCamera()->GetProjectionPtr());
	ShaderManager::setFloat(RE_CompPrimitive::shader, "objectColor", math::vec(1.0f, 1.0f, 1.0f));

	glEnable(GL_PROGRAM_POINT_SIZE);
	glPointSize(10.0f);

	glBindVertexArray(RE_CompPrimitive::VAO);
	glDrawArrays(GL_POINTS, 0, 1);

	glBindVertexArray(0);

	glPointSize(1.0f);
	glDisable(GL_PROGRAM_POINT_SIZE);

}

RE_CompLine::RE_CompLine(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader, math::vec origin, math::vec dest) : origin(origin), dest(dest), RE_CompPrimitive(C_LINE, game_obj, VAO, shader) {}

RE_CompLine::~RE_CompLine()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompLine::Draw()
{
	ShaderManager::use(RE_CompPrimitive::shader);
	ShaderManager::setFloat4x4(RE_CompPrimitive::shader, "model", RE_CompPrimitive::RE_Component::go->GetTransform()->GetShaderModel());
	ShaderManager::setFloat4x4(RE_CompPrimitive::shader, "view", App->editor->GetCamera()->GetViewPtr());
	ShaderManager::setFloat4x4(RE_CompPrimitive::shader, "projection", App->editor->GetCamera()->GetProjectionPtr());
	ShaderManager::setFloat(RE_CompPrimitive::shader, "objectColor", math::vec(1.0f, 0.0f, 0.0f));

	glLineWidth(2.0f);

	glBindVertexArray(RE_CompPrimitive::VAO);
	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);

	glLineWidth(1.0f);
}

RE_CompRay::RE_CompRay(RE_GameObject* game_obj, unsigned int shader, unsigned int VAO, math::vec origin, math::vec dir) : RE_CompLine(game_obj, NULL, NULL, origin, dir), RE_CompPrimitive(C_RAY, game_obj, VAO, shader) {}

RE_CompRay::~RE_CompRay()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompRay::Draw()
{
}

RE_CompTriangle::RE_CompTriangle(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader) : RE_CompPrimitive(C_TRIANGLE, game_obj, VAO, shader) {}

RE_CompTriangle::~RE_CompTriangle()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompTriangle::Draw()
{
	ShaderManager::use(RE_CompPrimitive::shader);
	ShaderManager::setFloat4x4(RE_CompPrimitive::shader, "model", RE_CompPrimitive::RE_Component::go->GetTransform()->GetShaderModel());
	ShaderManager::setFloat4x4(RE_CompPrimitive::shader, "view", App->editor->GetCamera()->GetViewPtr());
	ShaderManager::setFloat4x4(RE_CompPrimitive::shader, "projection", App->editor->GetCamera()->GetProjectionPtr());
	ShaderManager::setFloat(RE_CompPrimitive::shader, "objectColor", math::vec(1.0f, 0.0f, 0.0f));

	glBindVertexArray(RE_CompPrimitive::VAO);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
RE_CompPlane::RE_CompPlane(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader) : RE_CompPrimitive(C_PLANE, game_obj, VAO, shader) {}

RE_CompPlane::~RE_CompPlane()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompPlane::Draw()
{
	ShaderManager::use(RE_CompPrimitive::shader);
	ShaderManager::setFloat4x4(RE_CompPrimitive::shader, "model", RE_CompPrimitive::RE_Component::go->GetTransform()->GetShaderModel());
	ShaderManager::setFloat4x4(RE_CompPrimitive::shader, "view", App->editor->GetCamera()->GetViewPtr());
	ShaderManager::setFloat4x4(RE_CompPrimitive::shader, "projection", App->editor->GetCamera()->GetProjectionPtr());
	ShaderManager::setFloat(RE_CompPrimitive::shader, "objectColor", math::vec(1.0f, 0.0f, 0.0f));

	glBindVertexArray(RE_CompPrimitive::VAO);
	glDrawArrays(GL_LINES, 0, 400);
	glBindVertexArray(0);
}

RE_CompCube::RE_CompCube(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader, int triangle_count) : 
	RE_CompPrimitive(C_CUBE, game_obj, VAO, shader), triangle_count(triangle_count) 
{
	color = math::vec(1.0f, 0.15f, 0.15f);
}

RE_CompCube::~RE_CompCube()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompCube::Draw()
{
	unsigned int shader_id = (show_checkers ? App->scene->modelloading : RE_CompPrimitive::shader);

	ShaderManager::use(shader_id);
	ShaderManager::setFloat4x4(shader_id, "model", RE_CompPrimitive::RE_Component::go->GetTransform()->GetShaderModel());
	ShaderManager::setFloat4x4(shader_id, "view", App->editor->GetCamera()->GetViewPtr());
	ShaderManager::setFloat4x4(shader_id, "projection", App->editor->GetCamera()->GetProjectionPtr());

	if (!show_checkers)
	{
		// Apply Diffuse Color
		ShaderManager::setFloat(shader_id, "objectColor", color);

		// Draw
		glBindVertexArray(RE_CompPrimitive::VAO);
		glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_SHORT, 0);
	}
	else
	{
		// Apply Checkers Texture
		glActiveTexture(GL_TEXTURE0);
		ShaderManager::setUnsignedInt(shader_id, "texture_diffuse0", 0);
		glBindTexture(GL_TEXTURE_2D, App->scene->checkers_texture);

		// Draw
		glBindVertexArray(RE_CompPrimitive::VAO);
		glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_SHORT, 0);

		// Release Texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glBindVertexArray(0);
	ShaderManager::use(0);
}

RE_CompFustrum::RE_CompFustrum(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader) : RE_CompPrimitive(C_FUSTRUM, game_obj, VAO, shader) {}

RE_CompFustrum::~RE_CompFustrum()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompFustrum::Draw()
{

}

RE_CompSphere::RE_CompSphere(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader, int triangle_count)
	: RE_CompPrimitive(C_SPHERE, game_obj, VAO, shader),
	triangle_count(triangle_count)
{
	color = math::vec(1.0f, 0.15f, 0.15f);
}

RE_CompSphere::~RE_CompSphere()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompSphere::Draw()
{
	unsigned int shader_id = (show_checkers ? App->scene->modelloading : RE_CompPrimitive::shader);

	ShaderManager::use(shader_id);
	ShaderManager::setFloat4x4(shader_id, "model", RE_CompPrimitive::RE_Component::go->GetTransform()->GetShaderModel());
	ShaderManager::setFloat4x4(shader_id, "view", App->editor->GetCamera()->GetViewPtr());
	ShaderManager::setFloat4x4(shader_id, "projection", App->editor->GetCamera()->GetProjectionPtr());

	if (!show_checkers)
	{
		// Apply Diffuse Color
		ShaderManager::setFloat(shader_id, "objectColor", color);

		// Draw
		glBindVertexArray(RE_CompPrimitive::VAO);
		glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_SHORT, 0);
	}
	else
	{
		// Apply Checkers Texture
		glActiveTexture(GL_TEXTURE0);
		ShaderManager::setUnsignedInt(shader_id, "texture_diffuse0", 0);
		glBindTexture(GL_TEXTURE_2D, App->scene->checkers_texture);

		// Draw
		glBindVertexArray(RE_CompPrimitive::VAO);
		glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_SHORT, 0);

		// Release Texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glBindVertexArray(0);
	ShaderManager::use(0);
}

void RE_CompSphere::DrawProperties()
{
	if (ImGui::CollapsingHeader("Sphere"))
	{
		ImGui::Checkbox("Use checkers texture", &show_checkers);

		float p[3] = { color.x, color.y, color.z };
		if (ImGui::DragFloat3("Diffuse Color", p, 0.1f, 0.f, 255.f, "%.2f"))
			color = math::vec(p[0], p[1], p[2]);
	}
}

RE_CompCylinder::RE_CompCylinder(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader) : RE_CompPrimitive(C_CYLINDER, game_obj, VAO, shader) {}

RE_CompCylinder::~RE_CompCylinder()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompCylinder::Draw()
{
}

RE_CompCapsule::RE_CompCapsule(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader) : RE_CompPrimitive(C_CAPSULE, game_obj, VAO, shader) {}

RE_CompCapsule::~RE_CompCapsule()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompCapsule::Draw()
{
}
