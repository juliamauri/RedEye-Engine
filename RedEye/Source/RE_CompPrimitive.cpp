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

#ifndef PAR_SHAPES_IMPLEMENTATION
#define PAR_SHAPES_IMPLEMENTATION
#endif

#include "par_shapes.h"

RE_CompPrimitive::RE_CompPrimitive(ComponentType t, RE_GameObject* game_obj, unsigned int VAO, unsigned int shader) : VAO(VAO), type(t), shader(shader), RE_Component(t, game_obj) {}

RE_CompPrimitive::~RE_CompPrimitive()
{
}

ComponentType RE_CompPrimitive::GetType() const
{
	return type;
}

RE_CompGrid::RE_CompGrid(RE_GameObject* game_obj, unsigned int VAO, unsigned int shader) : RE_CompPrimitive(C_GRID, game_obj, VAO, shader) {}

RE_CompGrid::~RE_CompGrid()
{
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

RE_CompRock::RE_CompRock(RE_GameObject* game_obj, unsigned int shader, int _seed, int _subdivions)
	: RE_CompPrimitive(C_ROCK, game_obj, NULL, shader)
{
	if (_subdivions < 1) _subdivions = 1;
	if (_subdivions > 5) _subdivions = 5;
	RE_CompPrimitive::color = math::vec(1.0f, 0.15f, 0.15f);
	GenerateNewRock(seed = tmpSe = _seed, nsubdivisions = tmpSb = _subdivions);
}
RE_CompRock::RE_CompRock(const RE_CompRock& cmpRock, RE_GameObject* go)
	: RE_CompPrimitive(C_ROCK, go, NULL, cmpRock.RE_CompPrimitive::shader)
{
	RE_CompPrimitive::color = cmpRock.RE_CompPrimitive::color;
	GenerateNewRock(seed = tmpSe = cmpRock.seed, nsubdivisions = tmpSb = cmpRock.nsubdivisions);
}


RE_CompRock::~RE_CompRock()
{
	glDeleteVertexArrays(1, &(GLuint)RE_CompPrimitive::VAO);
	glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::VBO);
	glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::EBO);
}

void RE_CompRock::Draw()
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

void RE_CompRock::DrawProperties()
{
	if (ImGui::CollapsingHeader("Rock Primitive"))
	{
		ImGui::Checkbox("Use checkers texture", &show_checkers);

		ImGui::ColorEdit3("Diffuse Color", &RE_CompPrimitive::color[0]);

		ImGui::PushItemWidth(75.0f);
		if (ImGui::DragInt("Seed", &tmpSe, 1.0f))
		{
			if (seed != tmpSe) {
				seed = tmpSe;
				canChange = true;
			}
		}
		if (ImGui::DragInt("Num Subdivisions", &tmpSb, 1.0f, 1, 5))
		{
			if (tmpSb >= 3 && nsubdivisions != tmpSb) {
				nsubdivisions = tmpSb;
				canChange = true;
			}
			else if (tmpSb < 1) tmpSb = 1;
			else if (tmpSb > 5) tmpSb = 5;
		}
		ImGui::PopItemWidth();

		if (ImGui::Button("Apply")) GenerateNewRock(seed, nsubdivisions);
	}
}

unsigned int RE_CompRock::GetBinarySize() const
{
	return sizeof(float) * 3 + sizeof(int) * 2;
}

void RE_CompRock::SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources)
{
	node->PushFloatVector("color", RE_CompPrimitive::color);
	node->PushInt("seed", seed);
	node->PushInt("nsubdivisions", nsubdivisions);
}

void RE_CompRock::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources)
{
	size_t size = sizeof(int);
	memcpy(cursor, &seed, size);
	cursor += size;

	size = sizeof(int);
	memcpy(cursor, &nsubdivisions, size);
	cursor += size;

	size = sizeof(float) * 3;
	memcpy(cursor, &RE_CompPrimitive::color[0], size);
	cursor += size;
}

void RE_CompRock::GenerateNewRock(int seed, int subdivisions)
{
	if (canChange)
	{
		if (RE_CompPrimitive::VAO != 0) {
			glDeleteVertexArrays(1, &(GLuint)RE_CompPrimitive::VAO);
			glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::VBO);
			glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::EBO);
		}

		par_shapes_mesh* rock = par_shapes_create_rock(seed, subdivisions);

		float* points = new float[rock->npoints * 3];
		float* normals = new float[rock->npoints * 3];

		uint meshSize = 0;
		size_t size = rock->npoints * 3 * sizeof(float);
		uint stride = 0;

		memcpy(points, rock->points, size);
		meshSize += 3 * rock->npoints;
		stride += 3;

		memcpy(normals, rock->normals, size);
		meshSize += 3 * rock->npoints;
		stride += 3;

		stride *= sizeof(float);
		float* meshBuffer = new float[meshSize];
		float* cursor = meshBuffer;
		for (uint i = 0; i < rock->npoints; i++) {
			uint cursorSize = 3;
			size_t size = sizeof(float) * 3;

			memcpy(cursor, &points[i * 3], size);
			cursor += cursorSize;

			memcpy(cursor, &normals[i * 3], size);
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

		glGenBuffers(1, &(GLuint)RE_CompPrimitive::EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RE_CompPrimitive::EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, rock->ntriangles * sizeof(unsigned short) * 3, rock->triangles, GL_STATIC_DRAW);

		triangle_count = rock->ntriangles;

		par_shapes_free_mesh(rock);
		DEL_A(points);
		DEL_A(normals);
		DEL_A(meshBuffer);

		canChange = false;
	}
}

RE_CompParametric::RE_CompParametric(ComponentType t, const char* _name, RE_GameObject* game_obj, unsigned int shader, int _slice, int _stacks, bool _useRadius, float _radius, float _minR, float _maxR)
	: RE_CompPrimitive(t,game_obj, NULL, shader)
{
	if (_slice < 3) _slice = 3;
	if (_stacks < 3) _stacks = 3;
	pName = _name;
	tmpSl = slice = _slice;
	tmpSt = stacks = _stacks;
	useRadius = _useRadius;
	tmpR = radius = _radius;
	minR = _minR;
	maxR = maxR;
}

RE_CompParametric::~RE_CompParametric()
{
	glDeleteVertexArrays(1, &(GLuint)RE_CompPrimitive::VAO);
	glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::VBO);
	glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::EBO);
}

void RE_CompParametric::Draw()
{
	RE_GLCache::ChangeShader(RE_CompPrimitive::shader);
	RE_ShaderImporter::setFloat4x4(RE_CompPrimitive::shader, "model", RE_CompPrimitive::RE_Component::go->GetTransform()->GetShaderModel());

	if (!show_checkers)
	{
		// Apply Diffuse Color
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useColor", 1.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useTexture", 0.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "cdiffuse", RE_CompPrimitive::color);
	}
	else
	{
		// Apply Checkers Texture
		glActiveTexture(GL_TEXTURE0);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useColor", 0.0f);
		RE_ShaderImporter::setFloat(RE_CompPrimitive::shader, "useTexture", 1.0f);
		RE_ShaderImporter::setUnsignedInt(RE_CompPrimitive::shader, "tdiffuse", 0);
		RE_GLCache::ChangeTextureBind(App->internalResources->GetTextureChecker());
	}

	// Draw
	RE_GLCache::ChangeVAO(RE_CompPrimitive::VAO);
	glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_SHORT, 0);

}

void RE_CompParametric::DrawProperties()
{
	eastl::string title(pName);
	title += " Primitive";
	if (ImGui::CollapsingHeader(title.c_str()))
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

		if (useRadius && ImGui::DragFloat("Radius", &tmpR, 0.1f, minR, maxR))
		{
			if (tmpR >= 0.5f && tmpR <= 3.0f && radius != tmpR) {
				radius = tmpR;
				canChange = true;
			}
			else if (tmpR < minR) tmpR = minR;
			else if (tmpR > maxR) tmpR = maxR;
		}
		ImGui::PopItemWidth();

		if (canChange && ImGui::Button("Apply")) {
			GenerateParametric();
			canChange = false;
		}
	}
}

unsigned int RE_CompParametric::GetBinarySize() const
{
	return sizeof(float) * 4 + sizeof(int) * 2;
}

void RE_CompParametric::SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources)
{
	node->PushFloatVector("color", RE_CompPrimitive::color);
	node->PushInt("slices", slice);
	node->PushInt("stacks", stacks);
	node->PushFloat("radius", radius);
}

void RE_CompParametric::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources)
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

void RE_CompParametric::UploadParametric(par_shapes_mesh_s* param)
{
	if (RE_CompPrimitive::VAO != 0) {
		glDeleteVertexArrays(1, &(GLuint)RE_CompPrimitive::VAO);
		glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::VBO);
		glDeleteBuffers(1, &(GLuint)RE_CompPrimitive::EBO);
	}

	float* points = new float[param->npoints * 3];
	float* normals = new float[param->npoints * 3];
	float* texCoords = new float[param->npoints * 2];

	uint meshSize = 0;
	size_t size = param->npoints * 3 * sizeof(float);
	uint stride = 0;

	memcpy(points, param->points, size);
	meshSize += 3 * param->npoints;
	stride += 3;

	memcpy(normals, param->normals, size);
	meshSize += 3 * param->npoints;
	stride += 3;

	size = param->npoints * 2 * sizeof(float);
	memcpy(texCoords, param->tcoords, size);
	meshSize += 2 * param->npoints;
	stride += 2;

	stride *= sizeof(float);
	float* meshBuffer = new float[meshSize];
	float* cursor = meshBuffer;
	for (uint i = 0; i < param->npoints; i++) {
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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, param->ntriangles * sizeof(unsigned short) * 3, param->triangles, GL_STATIC_DRAW);

	triangle_count = param->ntriangles;

	DEL_A(points);
	DEL_A(normals);
	DEL_A(texCoords);
	DEL_A(meshBuffer);
}


RE_CompPlane::RE_CompPlane(RE_GameObject* game_obj, unsigned int shader, int _slice, int _stacks)
	: RE_CompParametric(C_PLANE, "Plane", game_obj, shader, _slice, _stacks)
{
	RE_CompPrimitive::color = math::vec(1.0f, 0.65f, 0.15f);
	GenerateParametric();
}
RE_CompPlane::RE_CompPlane(const RE_CompPlane& cmpPlane, RE_GameObject* go)
	: RE_CompParametric(C_PLANE, "Plane", go, cmpPlane.RE_CompPrimitive::shader, cmpPlane.RE_CompParametric::slice, cmpPlane.RE_CompParametric::stacks)
{
	RE_CompPrimitive::color = cmpPlane.RE_CompPrimitive::color;
	GenerateParametric();
}

RE_CompPlane::~RE_CompPlane()
{
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

void RE_CompPlane::GenerateParametric()
{
	par_shapes_mesh* plane = par_shapes_create_plane(slice, stacks);
	UploadParametric(plane);
	par_shapes_free_mesh(plane);
}

RE_CompSphere::RE_CompSphere(RE_GameObject* game_obj, unsigned int shader, int _slice, int _stacks)
	: RE_CompParametric(C_SPHERE, "Sphere", game_obj, shader, _slice, _stacks)
{
	RE_CompPrimitive::color = math::vec(1.0f, 0.65f, 0.15f);
	GenerateParametric();
}

RE_CompSphere::RE_CompSphere(const RE_CompSphere& cmpSphere, RE_GameObject* go)
	: RE_CompParametric(C_SPHERE, "Sphere", go, cmpSphere.RE_CompPrimitive::shader, cmpSphere.RE_CompParametric::slice, cmpSphere.RE_CompParametric::stacks)
{
	RE_CompPrimitive::color = math::vec(1.0f, 0.65f, 0.15f);
	GenerateParametric();
}

RE_CompSphere::~RE_CompSphere()
{
}

void RE_CompSphere::GenerateParametric()
{
	par_shapes_mesh* sphere = par_shapes_create_parametric_sphere(slice, stacks);
	UploadParametric(sphere);
	par_shapes_free_mesh(sphere);
}


RE_CompCylinder::RE_CompCylinder(RE_GameObject* game_obj, unsigned int shader, int _slice, int _stacks)
	: RE_CompParametric(C_CYLINDER, "Cylinder", game_obj, shader, _slice, _stacks) {
	RE_CompPrimitive::color = math::vec(1.0f, 0.15f, 0.15f);
	GenerateParametric();
}

RE_CompCylinder::RE_CompCylinder(const RE_CompCylinder& cmpCylinder, RE_GameObject* go)
	: RE_CompParametric(C_CYLINDER, "Cylinder", go, cmpCylinder.RE_CompPrimitive::shader, cmpCylinder.RE_CompParametric::slice, cmpCylinder.RE_CompParametric::stacks)
{
	RE_CompPrimitive::color = cmpCylinder.RE_CompPrimitive::color;
	GenerateParametric();
}

RE_CompCylinder::~RE_CompCylinder()
{
}

void RE_CompCylinder::GenerateParametric()
{
	par_shapes_mesh* cylinder = par_shapes_create_cylinder(slice, stacks);
	UploadParametric(cylinder);
	par_shapes_free_mesh(cylinder);
}

RE_CompHemiSphere::RE_CompHemiSphere(RE_GameObject* game_obj, unsigned int shader, int _slice, int _stacks)
	: RE_CompParametric(C_HEMISHPERE, "HemiSphere", game_obj, shader, _slice, _stacks) {
	RE_CompPrimitive::color = math::vec(1.0f, 0.15f, 0.15f);
	GenerateParametric();
}

RE_CompHemiSphere::RE_CompHemiSphere(const RE_CompHemiSphere& cmpHemiSphere, RE_GameObject* go)
	: RE_CompParametric(C_CYLINDER, "HemiSphere", go, cmpHemiSphere.RE_CompPrimitive::shader, cmpHemiSphere.RE_CompParametric::slice, cmpHemiSphere.RE_CompParametric::stacks)
{
	RE_CompPrimitive::color = cmpHemiSphere.RE_CompPrimitive::color;
	GenerateParametric();
}


RE_CompHemiSphere::~RE_CompHemiSphere()
{
}

void RE_CompHemiSphere::GenerateParametric()
{
	par_shapes_mesh* hemishpere = par_shapes_create_hemisphere(slice, stacks);
	UploadParametric(hemishpere);
	par_shapes_free_mesh(hemishpere);
}

RE_CompTorus::RE_CompTorus(RE_GameObject* game_obj, unsigned int shader, int _slice, int _stacks, float _radius)
	: RE_CompParametric(C_TORUS, "Torus", game_obj, shader, _slice, _stacks, true, _radius, 0.1f, 1.0f)
{
	RE_CompPrimitive::color = math::vec(1.0f, 0.15f, 0.15f);
	GenerateParametric();
}

RE_CompTorus::RE_CompTorus(const RE_CompTorus& cmpTorus, RE_GameObject* go)
	: RE_CompParametric(C_TORUS, "Torus", go, cmpTorus.RE_CompPrimitive::shader, cmpTorus.RE_CompParametric::slice, cmpTorus.RE_CompParametric::stacks, true, cmpTorus.RE_CompParametric::radius, 0.1f, 1.0f)

{
	RE_CompPrimitive::color = cmpTorus.RE_CompPrimitive::color;
	GenerateParametric();
}

RE_CompTorus::~RE_CompTorus()
{
}

void RE_CompTorus::GenerateParametric()
{
	par_shapes_mesh* torus = par_shapes_create_torus(slice, stacks, radius);
	UploadParametric(torus);
	par_shapes_free_mesh(torus);
}

RE_CompTrefoiKnot::RE_CompTrefoiKnot(RE_GameObject* game_obj, unsigned int shader, int _slice, int _stacks, float _radius)
	: RE_CompParametric(C_TREFOILKNOT,"Trefoil Knot", game_obj, shader, _slice, _stacks, true, _radius, 0.5f, 3.0f)
{
	RE_CompPrimitive::color = math::vec(1.0f, 0.15f, 0.15f);
	GenerateParametric();
}

RE_CompTrefoiKnot::RE_CompTrefoiKnot(const RE_CompTrefoiKnot& cmpTrefoiKnot, RE_GameObject* go)
	: RE_CompParametric(C_TREFOILKNOT, "Trefoil Knot", go, cmpTrefoiKnot.RE_CompPrimitive::shader, cmpTrefoiKnot.RE_CompParametric::slice, cmpTrefoiKnot.RE_CompParametric::stacks, true, cmpTrefoiKnot.RE_CompParametric::radius, 0.5f, 3.0f)

{
	RE_CompPrimitive::color = cmpTrefoiKnot.RE_CompPrimitive::color;
	GenerateParametric();
}

RE_CompTrefoiKnot::~RE_CompTrefoiKnot()
{
}

void RE_CompTrefoiKnot::GenerateParametric()
{
	par_shapes_mesh* trefoilknot = par_shapes_create_trefoil_knot(slice, stacks, radius);
	UploadParametric(trefoilknot);
	par_shapes_free_mesh(trefoilknot);
}