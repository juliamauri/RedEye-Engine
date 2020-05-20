#include "RE_CompPrimitive.h"

#include "SDL2/include/SDL.h"
#include "Glew/include/glew.h"
#include <gl/GL.h>
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleEditor.h"
#include "RE_ShaderImporter.h"
#include "RE_PrimitiveManager.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_CompCamera.h"
#include "ModuleScene.h"
#include "RE_FileSystem.h"
#include "RE_InternalResources.h"
#include "RE_ResourceManager.h"
#include "RE_GLCache.h"
#include "RE_Mesh.h"

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
	RE_GLCache::ChangeShader(RE_CompPrimitive::shader);
	RE_ShaderImporter::setFloat4x4(RE_CompPrimitive::shader, "model", RE_CompPrimitive::RE_Component::go->GetTransform()->GetShaderModel());
	RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useColor", 1.0f);
	RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useTexture", 0.0f);
	RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "cdiffuse", math::vec(1.0f, 1.0f, 1.0f));

	glEnable(GL_PROGRAM_POINT_SIZE);
	glPointSize(10.0f);

	RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);
	glDrawArrays(GL_POINTS, 0, 1);


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
	RE_GLCache::ChangeShader(RE_CompPrimitive::shader);
	RE_ShaderImporter::setFloat4x4(RE_CompPrimitive::shader, "model", RE_CompPrimitive::RE_Component::go->GetTransform()->GetShaderModel());
	RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useColor", 1.0f);
	RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useTexture", 0.0f);
	RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "cdiffuse", math::vec(1.0f, 0.0f, 0.0f));

	glLineWidth(2.0f);

	RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);
	glDrawArrays(GL_LINES, 0, 2);

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
	RE_GLCache::ChangeShader(RE_CompPrimitive::shader);
	RE_ShaderImporter::setFloat4x4(RE_CompPrimitive::shader, "model", RE_CompPrimitive::RE_Component::go->GetTransform()->GetShaderModel());
	RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useColor", 1.0f);
	RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useTexture", 0.0f);
	RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "cdiffuse", math::vec(1.0f, 0.0f, 0.0f));

	RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
}
RE_CompGrid::RE_CompGrid(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader) : RE_CompPrimitive(C_GRID, game_obj, VAO, shader) {}

RE_CompGrid::~RE_CompGrid()
{
	App->primitives->Rest(RE_CompPrimitive::type);
}

void RE_CompGrid::Draw()
{
	RE_GLCache::ChangeShader(RE_CompPrimitive::shader);
	RE_ShaderImporter::setFloat4x4(RE_CompPrimitive::shader, "model", RE_CompPrimitive::RE_Component::go->GetTransform()->GetShaderModel());
	RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useColor", 1.0f);
	RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useTexture", 0.0f);
	RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "cdiffuse", math::vec(1.0f, 0.0f, 0.0f));

	RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);
	glDrawArrays(GL_LINES, 0, 400);
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
}

void RE_CompCube::Draw()
{
	RE_GLCache::ChangeShader(RE_CompPrimitive::shader);
	RE_ShaderImporter::setFloat4x4(RE_CompPrimitive::shader, "model", RE_CompPrimitive::RE_Component::go->GetTransform()->GetShaderModel());

	if (!show_checkers)
	{
		// Apply Diffuse Color
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useColor", 1.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useTexture", 0.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "cdiffuse", RE_CompPrimitive::color);

		// Draw
		RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);
		glDrawElements(GL_TRIANGLES, triangle_count, GL_UNSIGNED_SHORT, 0);
	}
	else
	{
		// Apply Checkers Texture
		glActiveTexture(GL_TEXTURE0);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useColor", 0.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useTexture", 1.0f);
		RE_ShaderImporter::setUnsignedInt(RE_CompPrimitive::shader, "tdiffuse0", 0);
		RE_GLCache::ChangeTextureBind(App->internalResources->GetTextureChecker());

		// Draw
		RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);
		glDrawElements(GL_TRIANGLES, triangle_count, GL_UNSIGNED_SHORT, 0);
	}
}

void RE_CompCube::DrawProperties()
{
	if (ImGui::CollapsingHeader("Cube Primitive"))
	{
		ImGui::Checkbox("Use checkers texture", &show_checkers);

		ImGui::ColorEdit3("Diffuse Color", &RE_CompPrimitive::color[0]);
	}
}

unsigned int RE_CompCube::GetBinarySize() const
{
	return sizeof(float) * 3;
}

void RE_CompCube::SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources)
{
	node->PushFloatVector("color", RE_CompPrimitive::color);
}

void RE_CompCube::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources)
{
	size_t size = sizeof(float) * 3;
	memcpy(cursor, &RE_CompPrimitive::color[0], size);
	cursor += size;
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
	RE_GLCache::ChangeShader(RE_CompPrimitive::shader);
	RE_ShaderImporter::setFloat4x4(RE_CompPrimitive::shader, "model", RE_CompPrimitive::RE_Component::go->GetTransform()->GetShaderModel());

	if (!show_checkers)
	{
		// Apply Diffuse Color
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useColor", 1.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useTexture", 0.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "cdiffuse", RE_CompPrimitive::color);

		// Draw
		RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);
		glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_SHORT, 0);
	}
	else
	{
		// Apply Checkers Texture
		glActiveTexture(GL_TEXTURE0);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useColor", 0.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useTexture", 1.0f);
		RE_ShaderImporter::setUnsignedInt(RE_CompPrimitive::shader, "tdiffuse", 0);
		RE_GLCache::ChangeTextureBind(App->internalResources->GetTextureChecker());

		// Draw
		RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);
		glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_SHORT, 0);
	}
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

unsigned int RE_CompSphere::GetBinarySize() const
{
	return sizeof(float) * 3 + sizeof(int) * 2;
}

void RE_CompSphere::SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources)
{
	node->PushFloatVector("color", RE_CompPrimitive::color);
	node->PushInt("slices", slice);
	node->PushInt("stacks", stacks);
}

void RE_CompSphere::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources)
{
	size_t size = sizeof(int);
	memcpy(cursor, &slice, size);
	cursor += size;

	size = sizeof(int);
	memcpy(cursor, &stacks, size);
	cursor += size;

	size = sizeof(float) * 3;
	memcpy(cursor, &RE_CompPrimitive::color[0], size);
	cursor += size;
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

		float* points = new float[sphere->npoints * 3];
		float* normals = new float[sphere->npoints * 3];
		float* texCoords = new float[sphere->npoints * 2];

		uint meshSize = 0;
		size_t size = sphere->npoints * 3 * sizeof(float);
		uint stride = 0;

		memcpy(points, sphere->points, size);
		meshSize += 3 * sphere->npoints;
		stride += 3;

		memcpy(normals, sphere->normals, size);
		meshSize += 3 * sphere->npoints;
		stride += 3;

		size = sphere->npoints * 2 * sizeof(float);
		memcpy(texCoords, sphere->tcoords, size);
		meshSize += 2 * sphere->npoints;
		stride += 2;

		stride *= sizeof(float);
		float* meshBuffer = new float[meshSize];
		float* cursor = meshBuffer;
		for (uint i = 0; i < sphere->npoints; i++) {
			uint cursorSize = 3;
			size_t size = sizeof(float) * 3;

			memcpy(cursor, &points[i * 3], size);
			cursor += cursorSize;

			memcpy(cursor, &normals[i * 3], size);
			cursor += cursorSize;

			cursorSize = 2;
			size = sizeof(float) * 2;
			memcpy(cursor, &texCoords[i * 2], size);
			cursor += cursorSize;
		}


		glGenVertexArrays(1, &(GLuint)RE_CompPrimitive::VAO);
		RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);  

		glGenBuffers(1, &(GLuint)RE_CompPrimitive::VBO);
		glBindBuffer(GL_ARRAY_BUFFER, RE_CompPrimitive::VBO);
		glBufferData(GL_ARRAY_BUFFER, meshSize * sizeof(float), meshBuffer, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, NULL);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));

		glGenBuffers(1, &(GLuint)RE_CompPrimitive::EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RE_CompPrimitive::EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere->ntriangles * sizeof(unsigned short) * 3, sphere->triangles, GL_STATIC_DRAW);

		triangle_count = sphere->ntriangles;

		par_shapes_free_mesh(sphere);
		DEL_A(points);
		DEL_A(normals);
		DEL_A(texCoords);
		DEL_A(meshBuffer);

		canChange = false;
	}
}

RE_CompCylinder::RE_CompCylinder(RE_GameObject* game_obj, unsigned int shader, int _slice, int _stacks)
	: RE_CompPrimitive(C_CYLINDER, game_obj, NULL, shader) {
	if (_slice < 3) _slice = 3;
	if (_stacks < 3) _stacks = 3;
	RE_CompPrimitive::color = math::vec(1.0f, 0.15f, 0.15f);
	GenerateNewCylinder(slice = tmpSl = _slice, stacks = tmpSt = _stacks);
}

RE_CompCylinder::RE_CompCylinder(const RE_CompCylinder& cmpCylinder, RE_GameObject* go)
	: RE_CompPrimitive(C_CYLINDER, go, NULL, cmpCylinder.RE_CompPrimitive::shader)
{
	RE_CompPrimitive::color = cmpCylinder.RE_CompPrimitive::color;
	GenerateNewCylinder(slice = tmpSl = cmpCylinder.slice, stacks = tmpSt = cmpCylinder.stacks);
}

RE_CompCylinder::~RE_CompCylinder()
{
}

void RE_CompCylinder::Draw()
{
	RE_GLCache::ChangeShader(RE_CompPrimitive::shader);
	RE_ShaderImporter::setFloat4x4(RE_CompPrimitive::shader, "model", RE_CompPrimitive::RE_Component::go->GetTransform()->GetShaderModel());

	if (!show_checkers)
	{
		// Apply Diffuse Color
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useColor", 1.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useTexture", 0.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "cdiffuse", RE_CompPrimitive::color);

		// Draw
		RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);
		glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_SHORT, 0);
	}
	else
	{
		// Apply Checkers Texture
		glActiveTexture(GL_TEXTURE0);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useColor", 0.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useTexture", 1.0f);
		RE_ShaderImporter::setUnsignedInt(RE_CompPrimitive::shader, "tdiffuse", 0);
		RE_GLCache::ChangeTextureBind(App->internalResources->GetTextureChecker());

		// Draw
		RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);
		glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_SHORT, 0);
	}
}

void RE_CompCylinder::DrawProperties()
{
	if (ImGui::CollapsingHeader("Cylinder Primitive"))
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
		if (ImGui::DragInt("Stacks", &tmpSt, 1.0f, 3))
		{
			if (tmpSt >= 3 && stacks != tmpSt) {
				stacks = tmpSt;
				canChange = true;
			}
			else if (tmpSt < 3) tmpSt = 3;
		}
		ImGui::PopItemWidth();

		if (ImGui::Button("Apply")) GenerateNewCylinder(slice, stacks);
	}
}

unsigned int RE_CompCylinder::GetBinarySize() const
{
	return sizeof(float) * 3 + sizeof(int) * 2;
}

void RE_CompCylinder::SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources)
{
	node->PushFloatVector("color", RE_CompPrimitive::color);
	node->PushInt("slices", slice);
	node->PushInt("stacks", stacks);
}

void RE_CompCylinder::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources)
{
	size_t size = sizeof(int);
	memcpy(cursor, &slice, size);
	cursor += size;

	size = sizeof(int);
	memcpy(cursor, &stacks, size);
	cursor += size;

	size = sizeof(float) * 3;
	memcpy(cursor, &RE_CompPrimitive::color[0], size);
	cursor += size;
}

void RE_CompCylinder::GenerateNewCylinder(int slice, int stacks)
{
	if (canChange)
	{
		if (RE_CompPrimitive::VAO != 0) {
			glDeleteVertexArrays(1, &(GLuint)RE_CompPrimitive::VAO);
			glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::VBO);
			glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::EBO);
		}

		par_shapes_mesh* cylinder = par_shapes_create_cylinder(slice, stacks);

		float* points = new float[cylinder->npoints * 3];
		float* normals = new float[cylinder->npoints * 3];
		float* texCoords = new float[cylinder->npoints * 2];

		uint meshSize = 0;
		size_t size = cylinder->npoints * 3 * sizeof(float);
		uint stride = 0;

		memcpy(points, cylinder->points, size);
		meshSize += 3 * cylinder->npoints;
		stride += 3;

		memcpy(normals, cylinder->normals, size);
		meshSize += 3 * cylinder->npoints;
		stride += 3;

		size = cylinder->npoints * 2 * sizeof(float);
		memcpy(texCoords, cylinder->tcoords, size);
		meshSize += 2 * cylinder->npoints;
		stride += 2;

		stride *= sizeof(float);
		float* meshBuffer = new float[meshSize];
		float* cursor = meshBuffer;
		for (uint i = 0; i < cylinder->npoints; i++) {
			uint cursorSize = 3;
			size_t size = sizeof(float) * 3;

			memcpy(cursor, &points[i * 3], size);
			cursor += cursorSize;

			memcpy(cursor, &normals[i * 3], size);
			cursor += cursorSize;

			cursorSize = 2;
			size = sizeof(float) * 2;
			memcpy(cursor, &texCoords[i * 2], size);
			cursor += cursorSize;
		}


		glGenVertexArrays(1, &(GLuint)RE_CompPrimitive::VAO);
		RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);

		glGenBuffers(1, &(GLuint)RE_CompPrimitive::VBO);
		glBindBuffer(GL_ARRAY_BUFFER, RE_CompPrimitive::VBO);
		glBufferData(GL_ARRAY_BUFFER, meshSize * sizeof(float), meshBuffer, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, NULL);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));

		glGenBuffers(1, &(GLuint)RE_CompPrimitive::EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RE_CompPrimitive::EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylinder->ntriangles * sizeof(unsigned short) * 3, cylinder->triangles, GL_STATIC_DRAW);

		triangle_count = cylinder->ntriangles;

		par_shapes_free_mesh(cylinder);
		DEL_A(points);
		DEL_A(normals);
		DEL_A(texCoords);
		DEL_A(meshBuffer);

		canChange = false;
	}
}

RE_CompHemiSphere::RE_CompHemiSphere(RE_GameObject* game_obj, unsigned int shader, int _slice, int _stacks)
	: RE_CompPrimitive(C_CYLINDER, game_obj, NULL, shader) {
	if (_slice < 3) _slice = 3;
	if (_stacks < 3) _stacks = 3;
	RE_CompPrimitive::color = math::vec(1.0f, 0.15f, 0.15f);
	GenerateNewHemiSphere(slice = tmpSl = _slice, stacks = tmpSt = _stacks);
}

RE_CompHemiSphere::RE_CompHemiSphere(const RE_CompHemiSphere& cmpHemiSphere, RE_GameObject* go)
	: RE_CompPrimitive(C_CYLINDER, go, NULL, cmpHemiSphere.RE_CompPrimitive::shader)
{
	RE_CompPrimitive::color = cmpHemiSphere.RE_CompPrimitive::color;
	GenerateNewHemiSphere(slice = tmpSl = cmpHemiSphere.slice, stacks = tmpSt = cmpHemiSphere.stacks);
}


RE_CompHemiSphere::~RE_CompHemiSphere()
{
}

void RE_CompHemiSphere::Draw()
{
	RE_ShaderImporter::setFloat4x4(RE_CompPrimitive::shader, "model", RE_CompPrimitive::RE_Component::go->GetTransform()->GetShaderModel());

	if (!show_checkers)
	{
		// Apply Diffuse Color
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useColor", 1.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useTexture", 0.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "cdiffuse", RE_CompPrimitive::color);

		// Draw
		RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);
		glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_SHORT, 0);
	}
	else
	{
		// Apply Checkers Texture
		glActiveTexture(GL_TEXTURE0);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useColor", 0.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useTexture", 1.0f);
		RE_ShaderImporter::setUnsignedInt(RE_CompPrimitive::shader, "tdiffuse", 0);
		RE_GLCache::ChangeTextureBind(App->internalResources->GetTextureChecker());

		// Draw
		RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);
		glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_SHORT, 0);
	}
}

void RE_CompHemiSphere::DrawProperties()
{
	if (ImGui::CollapsingHeader("HemiSphere Primitive"))
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
		if (ImGui::DragInt("Stacks", &tmpSt, 1.0f, 3))
		{
			if (tmpSt >= 3 && stacks != tmpSt) {
				stacks = tmpSt;
				canChange = true;
			}
			else if (tmpSt < 3) tmpSt = 3;
		}
		ImGui::PopItemWidth();

		if (ImGui::Button("Apply")) GenerateNewHemiSphere(slice, stacks);
	}
}

unsigned int RE_CompHemiSphere::GetBinarySize() const
{
	return sizeof(float) * 3 + sizeof(int) * 2;
}

void RE_CompHemiSphere::SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources)
{
	node->PushFloatVector("color", RE_CompPrimitive::color);
	node->PushInt("slices", slice);
	node->PushInt("stacks", stacks);
}

void RE_CompHemiSphere::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources)
{
	size_t size = sizeof(int);
	memcpy(cursor, &slice, size);
	cursor += size;

	size = sizeof(int);
	memcpy(cursor, &stacks, size);
	cursor += size;

	size = sizeof(float) * 3;
	memcpy(cursor, &RE_CompPrimitive::color[0], size);
	cursor += size;
}

void RE_CompHemiSphere::GenerateNewHemiSphere(int slice, int stacks)
{
	if (canChange)
	{
		if (RE_CompPrimitive::VAO != 0) {
			glDeleteVertexArrays(1, &(GLuint)RE_CompPrimitive::VAO);
			glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::VBO);
			glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::EBO);
		}

		par_shapes_mesh* hemishpere = par_shapes_create_hemisphere(slice, stacks);

		float* points = new float[hemishpere->npoints * 3];
		float* normals = new float[hemishpere->npoints * 3];
		float* texCoords = new float[hemishpere->npoints * 2];

		uint meshSize = 0;
		size_t size = hemishpere->npoints * 3 * sizeof(float);
		uint stride = 0;

		memcpy(points, hemishpere->points, size);
		meshSize += 3 * hemishpere->npoints;
		stride += 3;

		memcpy(normals, hemishpere->normals, size);
		meshSize += 3 * hemishpere->npoints;
		stride += 3;

		size = hemishpere->npoints * 2 * sizeof(float);
		memcpy(texCoords, hemishpere->tcoords, size);
		meshSize += 2 * hemishpere->npoints;
		stride += 2;

		stride *= sizeof(float);
		float* meshBuffer = new float[meshSize];
		float* cursor = meshBuffer;
		for (uint i = 0; i < hemishpere->npoints; i++) {
			uint cursorSize = 3;
			size_t size = sizeof(float) * 3;

			memcpy(cursor, &points[i * 3], size);
			cursor += cursorSize;

			memcpy(cursor, &normals[i * 3], size);
			cursor += cursorSize;

			cursorSize = 2;
			size = sizeof(float) * 2;
			memcpy(cursor, &texCoords[i * 2], size);
			cursor += cursorSize;
		}


		glGenVertexArrays(1, &(GLuint)RE_CompPrimitive::VAO);
		RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);

		glGenBuffers(1, &(GLuint)RE_CompPrimitive::VBO);
		glBindBuffer(GL_ARRAY_BUFFER, RE_CompPrimitive::VBO);
		glBufferData(GL_ARRAY_BUFFER, meshSize * sizeof(float), meshBuffer, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, NULL);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));

		glGenBuffers(1, &(GLuint)RE_CompPrimitive::EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RE_CompPrimitive::EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, hemishpere->ntriangles * sizeof(unsigned short) * 3, hemishpere->triangles, GL_STATIC_DRAW);

		triangle_count = hemishpere->ntriangles;

		par_shapes_free_mesh(hemishpere);
		DEL_A(points);
		DEL_A(normals);
		DEL_A(texCoords);
		DEL_A(meshBuffer);

		canChange = false;
	}
}

RE_CompPlane::RE_CompPlane(RE_GameObject* game_obj, unsigned int shader, int _slice, int _stacks)
	: RE_CompPrimitive(C_PLANE, game_obj, NULL, shader)
{
	if (_slice < 3) _slice = 3;
	if (_stacks < 3) _stacks = 3;
	RE_CompPrimitive::color = math::vec(1.0f, 0.65f, 0.15f);
	GenerateNewPlane(slice = _slice, stacks = _stacks);
}
RE_CompPlane::RE_CompPlane(const RE_CompPlane& cmpPlane, RE_GameObject* go)
	: RE_CompPrimitive(C_PLANE, go, NULL, cmpPlane.RE_CompPrimitive::shader)
{
	RE_CompPrimitive::color = cmpPlane.RE_CompPrimitive::color;
	GenerateNewPlane(slice = cmpPlane.slice, stacks = cmpPlane.stacks);
}

RE_CompPlane::~RE_CompPlane()
{
	glDeleteVertexArrays(1, &(GLuint)RE_CompPrimitive::VAO);
	glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::VBO);
	glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::EBO);
}

void RE_CompPlane::Draw()
{
	RE_GLCache::ChangeShader(RE_CompPrimitive::shader);
	RE_ShaderImporter::setFloat4x4(RE_CompPrimitive::shader, "model", RE_CompPrimitive::RE_Component::go->GetTransform()->GetShaderModel());

	if (!show_checkers)
	{
		// Apply Diffuse Color
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useColor", 1.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useTexture", 0.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "cdiffuse", RE_CompPrimitive::color);

		// Draw
		RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);
		glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_SHORT, 0);
	}
	else
	{
		// Apply Checkers Texture
		glActiveTexture(GL_TEXTURE0);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useColor", 0.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useTexture", 1.0f);
		RE_ShaderImporter::setUnsignedInt(RE_CompPrimitive::shader, "tdiffuse0", 0);
		glBindTexture(GL_TEXTURE_2D, App->internalResources->GetTextureChecker());

		// Draw
		RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);
		glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_SHORT, 0);
	}
}

void RE_CompPlane::DrawProperties()
{
	if (ImGui::CollapsingHeader("Plane Primitive"))
	{
		ImGui::Checkbox("Use checkers texture", &show_checkers);

		ImGui::ColorEdit3("Diffuse Color", &RE_CompPrimitive::color[0]);

		ImGui::PushItemWidth(75.0f);
		static int tmpSl = slice;
		if (ImGui::DragInt("Slices", &tmpSl, 1.0f, 3))
		{
			if (slice != tmpSl && tmpSl >= 3) {
				slice = tmpSl;
				canChange = true;
			}
			else if (tmpSl < 3) { 
				canChange = (canChange || slice != tmpSl);
				slice = tmpSl = 3;
			}
		}

		static int tmpSt = stacks;
		if (ImGui::DragInt("Stacks", &tmpSt, 1.0f, 3))
		{
			if (tmpSt >= 3 && stacks != tmpSt) {
				stacks = tmpSt;
				canChange = true;
			}
			else if (tmpSt < 3) {
				canChange = (canChange || tmpSt != tmpSt);
				tmpSt = tmpSt = 3;
			}
		}
		ImGui::PopItemWidth();

		if (ImGui::Button("Apply")) GenerateNewPlane(slice, stacks);

		if (ImGui::Button("Convert To Mesh")) Event::Push(RE_EventType::PLANE_CHANGE_TO_MESH, App->scene, Cvar(GetGO()));
	}
}

unsigned int RE_CompPlane::GetBinarySize() const
{
	return sizeof(float) * 3 + sizeof(int) * 2;
}

void RE_CompPlane::SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources)
{
	node->PushFloatVector("color", RE_CompPrimitive::color);
	node->PushInt("slices", slice);
	node->PushInt("stacks", stacks);
}

void RE_CompPlane::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources)
{
	size_t size = sizeof(int);
	memcpy(cursor, &slice, size);
	cursor += size;

	size = sizeof(int);
	memcpy(cursor, &stacks, size);
	cursor += size;

	size = sizeof(float) * 3;
	memcpy(cursor, &RE_CompPrimitive::color[0], size);
	cursor += size;
}

const char* RE_CompPlane::TransformAsMeshResource()
{
	par_shapes_mesh* plane = par_shapes_create_plane(slice, stacks);

	float* points = new float[plane->npoints * 3];
	float* normals = new float[plane->npoints * 3];
	float* texCoords = new float[plane->npoints * 2];

	uint meshSize = 0;
	size_t size = plane->npoints * 3 * sizeof(float);
	uint stride = 0;

	memcpy(points, plane->points, size);
	meshSize += 3 * plane->npoints;
	stride += 3;

	memcpy(normals, plane->normals, size);
	meshSize += 3 * plane->npoints;
	stride += 3;

	size = plane->npoints * 2 * sizeof(float);
	memcpy(texCoords, plane->tcoords, size);
	meshSize += 2 * plane->npoints;
	stride += 2;

	eastl::vector<unsigned int> index;
	for (uint i = 0; i < plane->ntriangles * 3; i++)
		index.push_back(plane->triangles[i]);

	uint* indexA = new uint[plane->ntriangles * 3];
	memcpy(indexA, &index[0], index.size() * sizeof(uint));

	bool exists = false;
	RE_Mesh* newMesh = new RE_Mesh();
	newMesh->SetVerticesAndIndex(points, indexA, plane->npoints, plane->ntriangles, texCoords, normals);

	const char* meshMD5 = newMesh->CheckAndSave(&exists);
	if (!exists) {
		newMesh->SetName(eastl::string("Plane " + eastl::to_string(plane->ntriangles) + " triangles").c_str());
		newMesh->SetType(Resource_Type::R_MESH);
		App->resources->Reference(newMesh);
	}
	else
		DEL(newMesh);

	par_shapes_free_mesh(plane);
	return meshMD5;
}

void RE_CompPlane::GenerateNewPlane(int slice, int stacks)
{
	if (canChange) {
		if (RE_CompPrimitive::VAO != 0) {
			glDeleteVertexArrays(1, &(GLuint)RE_CompPrimitive::VAO);
			glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::VBO);
			glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::EBO);
		}

		par_shapes_mesh* plane = par_shapes_create_plane(slice, stacks);

		float* points = new float[plane->npoints * 3];
		float* normals = new float[plane->npoints * 3];
		float* texCoords = new float[plane->npoints * 2];

		uint meshSize = 0;
		size_t size = plane->npoints * 3 * sizeof(float);
		uint stride = 0;

		memcpy(points, plane->points, size);
		meshSize += 3 * plane->npoints;
		stride += 3;

		memcpy(normals, plane->normals, size);
		meshSize += 3 * plane->npoints;
		stride += 3;

		size = plane->npoints * 2 * sizeof(float);
		memcpy(texCoords, plane->tcoords, size);
		meshSize += 2 * plane->npoints;
		stride += 2;

		stride *= sizeof(float);
		float* meshBuffer = new float[meshSize];
		float* cursor = meshBuffer;
		for (uint i = 0; i < plane->npoints; i++) {
			uint cursorSize = 3;
			size_t size = sizeof(float) * 3;

			memcpy(cursor, &points[i * 3], size);
			cursor += cursorSize;

			memcpy(cursor, &normals[i * 3], size);
			cursor += cursorSize;

			cursorSize = 2;
			size = sizeof(float) * 2;
			memcpy(cursor, &texCoords[i * 2], size);
			cursor += cursorSize;
		}

		glGenVertexArrays(1, &(GLuint)RE_CompPrimitive::VAO);
		RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);

		glGenBuffers(1, &(GLuint)RE_CompPrimitive::VBO);
		glBindBuffer(GL_ARRAY_BUFFER, RE_CompPrimitive::VBO);
		glBufferData(GL_ARRAY_BUFFER, meshSize * sizeof(float), meshBuffer, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, NULL);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));

		glGenBuffers(1, &(GLuint)RE_CompPrimitive::EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RE_CompPrimitive::EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, plane->ntriangles * sizeof(unsigned short) * 3, plane->triangles, GL_STATIC_DRAW);

		triangle_count = plane->ntriangles;

		par_shapes_free_mesh(plane);
		DEL_A(points);
		DEL_A(normals);
		DEL_A(texCoords);
		DEL_A(meshBuffer);

		canChange = false;
	}
}

RE_CompTorus::RE_CompTorus(RE_GameObject* game_obj, unsigned int shader, int _slice, int _stacks, float _radius)
	: RE_CompPrimitive(C_TORUS, game_obj, NULL, shader)
{
	if (_slice < 3) _slice = 3;
	if (_stacks < 3) _stacks = 3;
	if (_radius < 0.1f) _radius = 0.1f;
	else if (_radius > 1.0f) _radius = 1.0f;
	RE_CompPrimitive::color = math::vec(1.0f, 0.15f, 0.15f);
	GenerateNewTorus(slice = tmpSl = _slice, stacks = tmpSt = _stacks, radius = tmpR = _radius);
}

RE_CompTorus::RE_CompTorus(const RE_CompTorus& cmpTorus, RE_GameObject* go)
	: RE_CompPrimitive(C_TORUS, go, NULL, cmpTorus.RE_CompPrimitive::shader)

{
	RE_CompPrimitive::color = cmpTorus.RE_CompPrimitive::color;
	GenerateNewTorus(slice = tmpSl = cmpTorus.slice, stacks = tmpSt = cmpTorus.stacks, radius = tmpR = cmpTorus.radius);
}

RE_CompTorus::~RE_CompTorus()
{
}

void RE_CompTorus::Draw()
{
	RE_ShaderImporter::setFloat4x4(RE_CompPrimitive::shader, "model", RE_CompPrimitive::RE_Component::go->GetTransform()->GetShaderModel());

	if (!show_checkers)
	{
		// Apply Diffuse Color
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useColor", 1.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useTexture", 0.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "cdiffuse", RE_CompPrimitive::color);

		// Draw
		RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);
		glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_SHORT, 0);
	}
	else
	{
		// Apply Checkers Texture
		glActiveTexture(GL_TEXTURE0);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useColor", 0.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useTexture", 1.0f);
		RE_ShaderImporter::setUnsignedInt(RE_CompPrimitive::shader, "tdiffuse", 0);
		RE_GLCache::ChangeTextureBind(App->internalResources->GetTextureChecker());

		// Draw
		RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);
		glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_SHORT, 0);
	}
}

void RE_CompTorus::DrawProperties()
{
	if (ImGui::CollapsingHeader("Torus Primitive"))
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
		if (ImGui::DragInt("Stacks", &tmpSt, 1.0f, 3))
		{
			if (tmpSt >= 3 && stacks != tmpSt) {
				stacks = tmpSt;
				canChange = true;
			}
			else if (tmpSt < 3) tmpSt = 3;
		}
		if (ImGui::DragFloat("Radius", &tmpR, 0.1f, 0.1f, 1.0f))
		{
			if (tmpR >= 0.1f && radius != tmpR) {
				radius = tmpR;
				canChange = true;
			}
			else if (tmpR < 0.1f) tmpR = 0.1f;
			else if (tmpR > 1.0f) tmpR = 1.0f;
		}
		ImGui::PopItemWidth();

		if (ImGui::Button("Apply")) GenerateNewTorus(slice, stacks, radius);
	}
}

unsigned int RE_CompTorus::GetBinarySize() const
{
	return sizeof(float) * 4 + sizeof(int) * 2;
}

void RE_CompTorus::SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources)
{
	node->PushFloatVector("color", RE_CompPrimitive::color);
	node->PushInt("slices", slice);
	node->PushInt("stacks", stacks);
	node->PushFloat("radius", radius);
}

void RE_CompTorus::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources)
{
	size_t size = sizeof(int);
	memcpy(cursor, &slice, size);
	cursor += size;

	size = sizeof(int);
	memcpy(cursor, &stacks, size);
	cursor += size;

	size = sizeof(float);
	memcpy(cursor, &radius, size);
	cursor += size;

	size = sizeof(float) * 3;
	memcpy(cursor, &RE_CompPrimitive::color[0], size);
	cursor += size;
}

void RE_CompTorus::GenerateNewTorus(int slice, int stacks, float radius)
{
	if (canChange)
	{
		if (RE_CompPrimitive::VAO != 0) {
			glDeleteVertexArrays(1, &(GLuint)RE_CompPrimitive::VAO);
			glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::VBO);
			glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::EBO);
		}

		par_shapes_mesh* torus = par_shapes_create_torus(slice, stacks, radius);

		float* points = new float[torus->npoints * 3];
		float* normals = new float[torus->npoints * 3];
		float* texCoords = new float[torus->npoints * 2];

		uint meshSize = 0;
		size_t size = torus->npoints * 3 * sizeof(float);
		uint stride = 0;

		memcpy(points, torus->points, size);
		meshSize += 3 * torus->npoints;
		stride += 3;

		memcpy(normals, torus->normals, size);
		meshSize += 3 * torus->npoints;
		stride += 3;

		size = torus->npoints * 2 * sizeof(float);
		memcpy(texCoords, torus->tcoords, size);
		meshSize += 2 * torus->npoints;
		stride += 2;

		stride *= sizeof(float);
		float* meshBuffer = new float[meshSize];
		float* cursor = meshBuffer;
		for (uint i = 0; i < torus->npoints; i++) {
			uint cursorSize = 3;
			size_t size = sizeof(float) * 3;

			memcpy(cursor, &points[i * 3], size);
			cursor += cursorSize;

			memcpy(cursor, &normals[i * 3], size);
			cursor += cursorSize;

			cursorSize = 2;
			size = sizeof(float) * 2;
			memcpy(cursor, &texCoords[i * 2], size);
			cursor += cursorSize;
		}


		glGenVertexArrays(1, &(GLuint)RE_CompPrimitive::VAO);
		RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);

		glGenBuffers(1, &(GLuint)RE_CompPrimitive::VBO);
		glBindBuffer(GL_ARRAY_BUFFER, RE_CompPrimitive::VBO);
		glBufferData(GL_ARRAY_BUFFER, meshSize * sizeof(float), meshBuffer, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, NULL);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));

		glGenBuffers(1, &(GLuint)RE_CompPrimitive::EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RE_CompPrimitive::EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, torus->ntriangles * sizeof(unsigned short) * 3, torus->triangles, GL_STATIC_DRAW);

		triangle_count = torus->ntriangles;

		par_shapes_free_mesh(torus);
		DEL_A(points);
		DEL_A(normals);
		DEL_A(texCoords);
		DEL_A(meshBuffer);

		canChange = false;
	}
}

RE_CompTrefoiKnot::RE_CompTrefoiKnot(RE_GameObject* game_obj, unsigned int shader, int _slice, int _stacks, float _radius)
	: RE_CompPrimitive(C_TREFOILKNOT, game_obj, NULL, shader)
{
	if (_slice < 3) _slice = 3;
	if (_stacks < 3) _stacks = 3;
	if (_radius < 0.5f) _radius = 0.5f;
	else if (_radius > 3.0f) _radius = 3.0f;
	RE_CompPrimitive::color = math::vec(1.0f, 0.15f, 0.15f);
	GenerateNewTrefoiKnot(slice = tmpSl = _slice, stacks = tmpSt = _stacks, radius = tmpR = _radius);
}

RE_CompTrefoiKnot::RE_CompTrefoiKnot(const RE_CompTrefoiKnot& cmpTrefoiKnot, RE_GameObject* go)
	: RE_CompPrimitive(C_TREFOILKNOT, go, NULL, cmpTrefoiKnot.RE_CompPrimitive::shader)

{
	RE_CompPrimitive::color = cmpTrefoiKnot.RE_CompPrimitive::color;
	GenerateNewTrefoiKnot(slice = tmpSl = cmpTrefoiKnot.slice, stacks = tmpSt = cmpTrefoiKnot.stacks, radius = tmpR = cmpTrefoiKnot.radius);
}

RE_CompTrefoiKnot::~RE_CompTrefoiKnot()
{
}

void RE_CompTrefoiKnot::Draw()
{
	RE_ShaderImporter::setFloat4x4(RE_CompPrimitive::shader, "model", RE_CompPrimitive::RE_Component::go->GetTransform()->GetShaderModel());

	if (!show_checkers)
	{
		// Apply Diffuse Color
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useColor", 1.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useTexture", 0.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "cdiffuse", RE_CompPrimitive::color);

		// Draw
		RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);
		glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_SHORT, 0);
	}
	else
	{
		// Apply Checkers Texture
		glActiveTexture(GL_TEXTURE0);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useColor", 0.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useTexture", 1.0f);
		RE_ShaderImporter::setUnsignedInt(RE_CompPrimitive::shader, "tdiffuse", 0);
		RE_GLCache::ChangeTextureBind(App->internalResources->GetTextureChecker());

		// Draw
		RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);
		glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_SHORT, 0);
	}
}

void RE_CompTrefoiKnot::DrawProperties()
{
	if (ImGui::CollapsingHeader("Trefoil Knot Primitive"))
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
		if (ImGui::DragInt("Stacks", &tmpSt, 1.0f, 3))
		{
			if (tmpSt >= 3 && stacks != tmpSt) {
				stacks = tmpSt;
				canChange = true;
			}
			else if (tmpSt < 3) tmpSt = 3;
		}
		if (ImGui::DragFloat("Radius", &tmpR, 0.1f, 0.5f, 3.0f))
		{
			if (tmpR >= 0.5f && tmpR <= 3.0f && radius != tmpR) {
				radius = tmpR;
				canChange = true;
			}
			else if (tmpR < 0.5f) tmpR = 0.5f;
			else if (tmpR > 3.0f) tmpR = 3.0f;
		}
		ImGui::PopItemWidth();

		if (ImGui::Button("Apply")) GenerateNewTrefoiKnot(slice, stacks, radius);
	}
}

unsigned int RE_CompTrefoiKnot::GetBinarySize() const
{
	return sizeof(float) * 4 + sizeof(int) * 2;
}

void RE_CompTrefoiKnot::SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources)
{
	node->PushFloatVector("color", RE_CompPrimitive::color);
	node->PushInt("slices", slice);
	node->PushInt("stacks", stacks);
	node->PushFloat("radius", radius);
}

void RE_CompTrefoiKnot::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources)
{
	size_t size = sizeof(int);
	memcpy(cursor, &slice, size);
	cursor += size;

	size = sizeof(int);
	memcpy(cursor, &stacks, size);
	cursor += size;

	size = sizeof(float);
	memcpy(cursor, &radius, size);
	cursor += size;

	size = sizeof(float) * 3;
	memcpy(cursor, &RE_CompPrimitive::color[0], size);
	cursor += size;
}

void RE_CompTrefoiKnot::GenerateNewTrefoiKnot(int slice, int stacks, float radius)
{
	if (canChange)
	{
		if (RE_CompPrimitive::VAO != 0) {
			glDeleteVertexArrays(1, &(GLuint)RE_CompPrimitive::VAO);
			glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::VBO);
			glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::EBO);
		}

		par_shapes_mesh* torus = par_shapes_create_trefoil_knot(slice, stacks, radius);

		float* points = new float[torus->npoints * 3];
		float* normals = new float[torus->npoints * 3];
		float* texCoords = new float[torus->npoints * 2];

		uint meshSize = 0;
		size_t size = torus->npoints * 3 * sizeof(float);
		uint stride = 0;

		memcpy(points, torus->points, size);
		meshSize += 3 * torus->npoints;
		stride += 3;

		memcpy(normals, torus->normals, size);
		meshSize += 3 * torus->npoints;
		stride += 3;

		size = torus->npoints * 2 * sizeof(float);
		memcpy(texCoords, torus->tcoords, size);
		meshSize += 2 * torus->npoints;
		stride += 2;

		stride *= sizeof(float);
		float* meshBuffer = new float[meshSize];
		float* cursor = meshBuffer;
		for (uint i = 0; i < torus->npoints; i++) {
			uint cursorSize = 3;
			size_t size = sizeof(float) * 3;

			memcpy(cursor, &points[i * 3], size);
			cursor += cursorSize;

			memcpy(cursor, &normals[i * 3], size);
			cursor += cursorSize;

			cursorSize = 2;
			size = sizeof(float) * 2;
			memcpy(cursor, &texCoords[i * 2], size);
			cursor += cursorSize;
		}


		glGenVertexArrays(1, &(GLuint)RE_CompPrimitive::VAO);
		RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);

		glGenBuffers(1, &(GLuint)RE_CompPrimitive::VBO);
		glBindBuffer(GL_ARRAY_BUFFER, RE_CompPrimitive::VBO);
		glBufferData(GL_ARRAY_BUFFER, meshSize * sizeof(float), meshBuffer, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, NULL);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));

		glGenBuffers(1, &(GLuint)RE_CompPrimitive::EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RE_CompPrimitive::EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, torus->ntriangles * sizeof(unsigned short) * 3, torus->triangles, GL_STATIC_DRAW);

		triangle_count = torus->ntriangles;

		par_shapes_free_mesh(torus);
		DEL_A(points);
		DEL_A(normals);
		DEL_A(texCoords);
		DEL_A(meshBuffer);

		canChange = false;
	}
}
