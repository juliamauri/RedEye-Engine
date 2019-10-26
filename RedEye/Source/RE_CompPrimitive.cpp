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
#include "FileSystem.h"

#include "par_shapes.h"

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
	RE_CompPrimitive::color = math::vec(1.0f, 0.15f, 0.15f);
}

RE_CompCube::RE_CompCube(const RE_CompCube & cmpCube, RE_GameObject * go) :
	RE_CompPrimitive(C_CUBE, go, cmpCube.RE_CompPrimitive::VAO, cmpCube.RE_CompPrimitive::shader)
{
	RE_CompPrimitive::color = cmpCube.RE_CompPrimitive::color;
	triangle_count = cmpCube.triangle_count;
	RE_CompPrimitive::VAO = App->primitives->CheckCubeVAO();
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
		ShaderManager::setFloat(shader_id, "objectColor", RE_CompPrimitive::color);

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

void RE_CompCube::DrawProperties()
{
	if (ImGui::CollapsingHeader("Cube Primitive"))
	{
		ImGui::Checkbox("Use checkers texture", &show_checkers);

		ImGui::ColorEdit3("Diffuse Color", &RE_CompPrimitive::color[0]);
	}
}

void RE_CompCube::Serialize(JSONNode * node, rapidjson::Value * comp_array)
{
	rapidjson::Value val(rapidjson::kObjectType);
	val.AddMember(rapidjson::Value::StringRefType("type"), rapidjson::Value().SetInt((int)ComponentType::C_CUBE), node->GetDocument()->GetAllocator());

	rapidjson::Value float_array(rapidjson::kArrayType);

	float_array.PushBack(RE_CompPrimitive::color.x, node->GetDocument()->GetAllocator()).PushBack(RE_CompPrimitive::color.y, node->GetDocument()->GetAllocator()).PushBack(RE_CompPrimitive::color.z, node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("color"), float_array.Move(), node->GetDocument()->GetAllocator());

	comp_array->PushBack(val, node->GetDocument()->GetAllocator());
}

RE_CompFustrum::RE_CompFustrum(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader) : RE_CompPrimitive(C_FUSTRUM, game_obj, VAO, shader) {}

RE_CompFustrum::~RE_CompFustrum()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompFustrum::Draw()
{

}

RE_CompSphere::RE_CompSphere(RE_GameObject* game_obj, unsigned int shader, int _slice, int _stacks)
	: RE_CompPrimitive(C_SPHERE, game_obj, NULL, shader)
{
	if (_slice < 3) _slice = 3;
	if (_stacks < 3) _stacks = 3;
	RE_CompPrimitive::color = math::vec(1.0f, 0.15f, 0.15f);
	GenerateNewSphere(slice = tmpSl = _slice, stacks = tmpSt = _stacks);
}

RE_CompSphere::RE_CompSphere(const RE_CompSphere & cmpSphere, RE_GameObject * go)
	: RE_CompPrimitive(C_SPHERE, go, NULL, cmpSphere.RE_CompPrimitive::shader)
{
	RE_CompPrimitive::color = cmpSphere.RE_CompPrimitive::color;
	GenerateNewSphere(slice = tmpSl = cmpSphere.slice, stacks = tmpSt = cmpSphere.stacks);
}

RE_CompSphere::~RE_CompSphere()
{
	glDeleteVertexArrays(1, &(GLuint)RE_CompPrimitive::VAO);
	glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::VBO);
	glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::EBO);
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
		ShaderManager::setFloat(shader_id, "objectColor", RE_CompPrimitive::color);

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
	if (ImGui::CollapsingHeader("Sphere Primitive"))
	{
		ImGui::Checkbox("Use checkers texture", &show_checkers);

		ImGui::ColorEdit3("Diffuse Color", &RE_CompPrimitive::color[0]);

		ImGui::PushItemWidth(75.0f);
		if (ImGui::DragInt("Slices", &tmpSl, 1.0f, 3))
		{
			if (slice != tmpSl && tmpSl >= 3) {
				slice = tmpSl;
				canChange = true;
			}
			else if (tmpSl < 3) tmpSl = 3;
		}
		if(ImGui::DragInt("Stacks", &tmpSt, 1.0f, 3))
		{
			if (tmpSt >= 3 && stacks != tmpSt) {
				stacks = tmpSt;
				canChange = true;
			}
			else if (tmpSt < 3) tmpSt = 3;
		}
		ImGui::PopItemWidth();

		if (ImGui::Button("Apply")) GenerateNewSphere(slice, stacks);
	}
}

void RE_CompSphere::Serialize(JSONNode * node, rapidjson::Value * comp_array)
{
	rapidjson::Value val(rapidjson::kObjectType);
	val.AddMember(rapidjson::Value::StringRefType("type"), rapidjson::Value().SetInt((int)ComponentType::C_SPHERE), node->GetDocument()->GetAllocator());

	rapidjson::Value float_array(rapidjson::kArrayType);

	float_array.PushBack(RE_CompPrimitive::color.x, node->GetDocument()->GetAllocator()).PushBack(RE_CompPrimitive::color.y, node->GetDocument()->GetAllocator()).PushBack(RE_CompPrimitive::color.z, node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("color"), float_array.Move(), node->GetDocument()->GetAllocator());

	val.AddMember(rapidjson::Value::StringRefType("slices"), rapidjson::Value().SetInt((int)slice), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("stacks"), rapidjson::Value().SetInt((int)stacks), node->GetDocument()->GetAllocator());

	comp_array->PushBack(val, node->GetDocument()->GetAllocator());
}

void RE_CompSphere::GenerateNewSphere(int _slice, int _stacks)
{
	if (canChange)
	{
		if (RE_CompPrimitive::VAO != 0) {
			glDeleteVertexArrays(1, &(GLuint)RE_CompPrimitive::VAO);
			glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::VBO);
			glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::EBO);
		}

		par_shapes_mesh* sphere = par_shapes_create_parametric_sphere(slice, stacks);

		glGenVertexArrays(1, &(GLuint)RE_CompPrimitive::VAO);
		glBindVertexArray(RE_CompPrimitive::VAO);

		glGenBuffers(1, &(GLuint)RE_CompPrimitive::VBO);
		glBindBuffer(GL_ARRAY_BUFFER, RE_CompPrimitive::VBO);
		glBufferData(GL_ARRAY_BUFFER, sphere->npoints * sizeof(float) * 3, sphere->points, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &(GLuint)RE_CompPrimitive::EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RE_CompPrimitive::EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere->ntriangles * sizeof(unsigned short) * 3, sphere->triangles, GL_STATIC_DRAW);

		triangle_count = sphere->ntriangles;

		par_shapes_free_mesh(sphere);

		canChange = false;
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
