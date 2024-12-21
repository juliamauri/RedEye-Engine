#include <MGL/Math/float3.h>
#include <EASTL/string.h>

#include "RE_CompPrimitive.h"

#include "RE_Memory.h"
#include "RE_Math.h"
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModuleEditor.h"
#include "RE_Json.h"
#include "RE_ShaderImporter.h"
#include "RE_InternalResources.h"
#include "RE_ResourceManager.h"
#include "RE_PrimitiveManager.h"
#include "RE_GLCache.h"
#include "RE_Mesh.h"
#include "RE_Shader.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"

#include <ImGui/imgui.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <gl/GL.h>

#include <par_shapes.h>

RE_CompPrimitive::RE_CompPrimitive(RE_Component::Type t) : RE_Component(t)
{
	color = math::vec(1.0f, 0.15f, 0.15f);
}

void RE_CompPrimitive::SimpleDraw() const
{
	RE_GLCache::ChangeVAO(VAO);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(triangle_count) * 3, GL_UNSIGNED_SHORT, 0);
	RE_GLCache::ChangeVAO(0);
}

bool RE_CompPrimitive::CheckFaceCollision(const math::Ray& ray, float& distance) const
{
	// TODO Rub: Primitive raycast checking
	return false;
}

void RE_CompPrimitive::SetColor(float r, float g, float b) { color.Set(r, g, b); }
void RE_CompPrimitive::SetColor(math::vec nColor) { color = nColor; }
unsigned int RE_CompPrimitive::GetVAO() const { return VAO; }
size_t RE_CompPrimitive::GetTriangleCount() const { return triangle_count; }

void RE_CompPrimitive::UnUseResources() { RE_SCENE->primitives->UnUsePrimitive(type, primID); }

void RE_CompPrimitive::SerializeParticleJson(RE_Json* node) const { DEL(node) }
void RE_CompPrimitive::DeserializeParticleJson(RE_Json* node) { DEL(node) }

///////   Grid   ////////////////////////////////////////////

void RE_CompGrid::GridSetUp(int newD)
{
	if (!useParent && !transform) {
		transform = new RE_CompTransform();
		transform->SetParent(0ull);
	}

	if (newD < 0) newD = 10;
	
	tmpSb = divisions = newD;

	primID = static_cast<int>(type) + divisions;
	auto mD = RE_SCENE->primitives->GetPrimitiveMeshData(this, primID);
	VAO = mD.first;
}

void RE_CompGrid::CopySetUp(GameObjectsPool* pool, RE_Component* _copy, const GO_UID parent)
{
	pool_gos = pool;
	if (useParent = (go = parent)) pool_gos->AtPtr(go)->ReportComponent(id, type);

	RE_CompGrid* copy = dynamic_cast<RE_CompGrid*>(_copy);
	color = copy->color;
	GridSetUp(copy->divisions);
}

void RE_CompGrid::Draw() const
{
	unsigned int shader = dynamic_cast<RE_Shader*>(RE_RES->At(RE_InternalResources::GetDefaultShader()))->GetID();
	RE_GLCache::ChangeShader(shader);
	RE_ShaderImporter::setFloat4x4(shader, "model", GetTransformPtr()->GetGlobalMatrixPtr());
	RE_ShaderImporter::setFloat(shader, "useColor", 1.0f);
	RE_ShaderImporter::setFloat(shader, "useTexture", 0.0f);
	RE_ShaderImporter::setFloat(shader, "cdiffuse", color);

	RE_GLCache::ChangeVAO(VAO);
	glDrawArrays(GL_LINES, 0, (divisions * 8) + 4);
	RE_GLCache::ChangeVAO(0);
}

void RE_CompGrid::DrawProperties()
{
	if (ImGui::CollapsingHeader("Grid Primitive"))
	{
		ImGui::ColorEdit3("Diffuse Color", &RE_CompPrimitive::color[0]);

		static bool apply = false;
		if (ImGui::DragInt("Num Subdivisions", &tmpSb, 1.0f, 1))
			if(tmpSb != divisions) apply = true;

		if (apply && ImGui::Button("Apply"))
			GridSetUp(tmpSb);
	}
}

RE_CompTransform* RE_CompGrid::GetTransformPtr() const
{
	return (useParent) ? GetGOCPtr()->GetTransformPtr() : transform;
}

size_t RE_CompGrid::GetBinarySize() const
{
	return sizeof(float) * 3u + sizeof(int);
}

void RE_CompGrid::SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources) const
{
	node->PushFloatVector("color", color);
	node->Push("divisions", divisions);
}

void RE_CompGrid::DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources)
{
	color = node->PullFloatVector("color", color);
	GridSetUp(node->PullInt("divisions", divisions));
}

void RE_CompGrid::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const
{
	size_t size = sizeof(float) * 3;
	memcpy(cursor, color.ptr(), size);
	cursor += size;

	size = sizeof(int);
	memcpy(cursor, &divisions, size);
	cursor += size;
}

void RE_CompGrid::DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources)
{
	size_t size = sizeof(float) * 3;
	memcpy(&color[0], cursor, size);
	cursor += size;

	size = sizeof(int);
	memcpy(&divisions, cursor, size);
	cursor += size;

	GridSetUp(divisions);
}

float RE_CompGrid::GetDistance() const
{
	return distance;
}

///////   Rock   ////////////////////////////////////////////

void RE_CompRock::RockSetUp(int _seed, int _subdivions)
{
	GenerateNewRock(_seed, RE_Math::Cap(_subdivions, 1, 5));
}

void RE_CompRock::CopySetUp(GameObjectsPool* pool, RE_Component* _copy, const GO_UID parent)
{
	pool_gos = pool;
	if (useParent = (go = parent)) pool_gos->AtPtr(go)->ReportComponent(id, type);

	RE_CompRock* copy = dynamic_cast<RE_CompRock*>(_copy);
	color = copy->color;
	RockSetUp(copy->seed, copy->nsubdivisions);
}

void RE_CompRock::Draw() const
{
	unsigned int shader = dynamic_cast<RE_Shader*>(RE_RES->At(RE_InternalResources::GetDefaultShader()))->GetID();
	RE_GLCache::ChangeShader(shader);
	RE_ShaderImporter::setFloat4x4(shader, "model", GetGOCPtr()->GetTransformPtr()->GetGlobalMatrixPtr());

	// Apply Diffuse Color
	RE_ShaderImporter::setFloat(shader, "useColor", 1.0f);
	RE_ShaderImporter::setFloat(shader, "useTexture", 0.0f);
	RE_ShaderImporter::setFloat(shader, "cdiffuse", color);

	// Draw
	RE_GLCache::ChangeVAO(VAO);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(triangle_count) * 3, GL_UNSIGNED_SHORT, 0);
	RE_GLCache::ChangeVAO(0);
}

void RE_CompRock::DrawProperties()
{
	if (ImGui::CollapsingHeader("Rock Primitive"))
	{
		int tmpSe = seed, tmpSb = nsubdivisions;

		ImGui::Text("Can't checker because doesn't support Texture Coords");
		ImGui::ColorEdit3("Diffuse Color", &RE_CompPrimitive::color[0]);
		ImGui::PushItemWidth(75.0f);

		if (ImGui::DragInt("Seed", &tmpSe, 1.0f) && seed != tmpSe)
		{
			seed = tmpSe;
			canChange = true;
		}

		if (ImGui::DragInt("Num Subdivisions", &tmpSb, 1.0f, 1, 5))
		{
			tmpSb = RE_Math::Cap(tmpSb, 1, 5);
			if (nsubdivisions != tmpSb)
			{
				nsubdivisions = tmpSb;
				canChange = true;
			}
		}

		ImGui::PopItemWidth();
		if (canChange && ImGui::Button("Apply")) GenerateNewRock(seed, nsubdivisions);
	}
}

bool RE_CompRock::DrawPrimPropierties()
{
	bool ret = false;
	int tmpSe = seed, tmpSb = nsubdivisions;

	//ImGui::Text("Can't use texture because doesn't support Texture Coords");
	if (ImGui::DragInt("Seed", &tmpSe, 1.0f) && seed != tmpSe)
	{
		seed = tmpSe;
		canChange = true;
		ret = true;
	}

	if (ImGui::DragInt("Num Subdivisions", &tmpSb, 1.0f, 1, 5))
	{
		tmpSb = RE_Math::Cap(tmpSb, 1, 5);
		if ( nsubdivisions != tmpSb)
		{
			nsubdivisions = tmpSb;
			canChange = true;
			ret = true;
		}
	}

	if (canChange && ImGui::Button("Apply")) GenerateNewRock(seed, nsubdivisions);
	return ret;
}

size_t RE_CompRock::GetBinarySize() const
{
	return sizeof(float) * 3 + sizeof(int) * 2;
}

void RE_CompRock::SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources) const
{
	node->PushFloatVector("color", color);
	node->Push("seed", seed);
	node->Push("nsubdivisions", nsubdivisions);
}

void RE_CompRock::DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources)
{
	color = node->PullFloatVector("color", { 1.0f,1.0f,1.0f });
	RockSetUp(node->PullInt("seed", seed), node->PullInt("nsubdivisions", nsubdivisions));
}

void RE_CompRock::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const
{
	size_t size = sizeof(float) * 3;
	memcpy(cursor, color.ptr(), size);
	cursor += size;

	size = sizeof(int);
	memcpy(cursor, &seed, size);
	cursor += size;

	size = sizeof(int);
	memcpy(cursor, &nsubdivisions, size);
	cursor += size;
}

void RE_CompRock::DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources)
{
	size_t size = sizeof(float) * 3;
	memcpy(&color[0], cursor, size);
	cursor += size;

	size = sizeof(int);
	memcpy(&seed, cursor, size);
	cursor += size;

	size = sizeof(int);
	memcpy(&nsubdivisions, cursor, size);
	cursor += size;

	RockSetUp(seed, nsubdivisions);
}

size_t RE_CompRock::GetParticleBinarySize() const
{
	return sizeof(int) * 2;
}

void RE_CompRock::SerializeParticleJson(RE_Json* node) const
{
	node->Push("seed", seed);
	node->Push("nsubdivisions", nsubdivisions);
	DEL(node)
}

void RE_CompRock::DeserializeParticleJson(RE_Json* node)
{
	RockSetUp(node->PullInt("seed", seed), node->PullInt("nsubdivisions", nsubdivisions));
	DEL(node)
}

void RE_CompRock::SerializeParticleBinary(char*& cursor) const
{
	size_t size = sizeof(int);
	memcpy(cursor, &seed, size);
	cursor += size;

	size = sizeof(int);
	memcpy(cursor, &nsubdivisions, size);
	cursor += size;
}

void RE_CompRock::DeserializeParticleBinary(char*& cursor)
{
	size_t size = sizeof(int);
	memcpy(&seed, cursor, size);
	cursor += size;

	size = sizeof(int);
	memcpy(&nsubdivisions, cursor, size);
	cursor += size;

	RockSetUp(seed, nsubdivisions);
}

void RE_CompRock::GenerateNewRock(int s, int subdivisions)
{
	seed = s;
	nsubdivisions = subdivisions;

	if (primID != -1) RE_SCENE->primitives->UnUsePrimitive(type, primID);
	primID = seed * nsubdivisions * static_cast<int>(RE_Component::Type::ROCK);
	auto mD = RE_SCENE->primitives->GetPrimitiveMeshData(this, primID);

	VAO = mD.first;
	triangle_count = mD.second;

	canChange = false;
}

///////   Platonic   ////////////////////////////////////////////

void RE_CompPlatonic::PlatonicSetUp()
{
	auto mD = RE_SCENE->primitives->GetPrimitiveMeshData(this);
	VAO = mD.first;
	triangle_count = mD.second;
}

void RE_CompPlatonic::CopySetUp(GameObjectsPool* pool, RE_Component* _copy, const GO_UID parent)
{
	pool_gos = pool;
	if (useParent = (go = parent)) pool_gos->AtPtr(go)->ReportComponent(id, type);

	RE_CompPlatonic* copy = dynamic_cast<RE_CompPlatonic*>(_copy);
	color = copy->color;

	PlatonicSetUp();
}

void RE_CompPlatonic::Draw() const
{
	unsigned int shader = dynamic_cast<RE_Shader*>(RE_RES->At(RE_InternalResources::GetDefaultShader()))->GetID();
	RE_GLCache::ChangeShader(shader);
	RE_ShaderImporter::setFloat4x4(shader, "model", GetGOCPtr()->GetTransformPtr()->GetGlobalMatrixPtr());

	// Apply Diffuse Color
	RE_ShaderImporter::setFloat(shader, "useColor", 1.0f);
	RE_ShaderImporter::setFloat(shader, "useTexture", 0.0f);
	RE_ShaderImporter::setFloat(shader, "cdiffuse", color);
	RE_ShaderImporter::setFloat(shader, "cspecular", math::vec::zero);
	RE_ShaderImporter::setFloat(shader, "shininess", 1.0f);
	RE_ShaderImporter::setFloat(shader, "opacity", 1.0f);

	// Draw
	RE_GLCache::ChangeVAO(VAO);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(triangle_count) * 3, GL_UNSIGNED_SHORT, 0);
	RE_GLCache::ChangeVAO(0);
}

void RE_CompPlatonic::DrawProperties()
{
	if (ImGui::CollapsingHeader((pName + " Primitive").c_str()))
	{
		ImGui::Text("Can't checker because don't support Texture Coords");
		ImGui::ColorEdit3("Diffuse Color", &color[0]);
	}
}

bool RE_CompPlatonic::DrawPrimPropierties()
{
	return false;
}

size_t RE_CompPlatonic::GetBinarySize() const
{
	return sizeof(float) * 3;
}

void RE_CompPlatonic::SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources) const
{
	node->PushFloatVector("color", color);
}

void RE_CompPlatonic::DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources)
{
	color = node->PullFloatVector("color", color);
	PlatonicSetUp();
}

void RE_CompPlatonic::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const
{
	size_t size = sizeof(float) * 3;
	memcpy(cursor, color.ptr(), size);
	cursor += size;
}

void RE_CompPlatonic::DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources)
{
	size_t size = sizeof(float) * 3;
	memcpy(&color[0], cursor, size);
	cursor += size;

	PlatonicSetUp();
}

///////   Parametric   ////////////////////////////////////////////
RE_CompParametric::RE_CompParametric(RE_Component::Type t, const char* _name) : RE_CompPrimitive(t), name(_name) {}
RE_CompParametric::~RE_CompParametric() { }

void RE_CompParametric::ParametricSetUp(int _slices, int _stacks, float _radius)
{
	target_slices = slices = RE_Math::Cap(_slices, 3, INT_MAX);
	target_stacks = stacks = RE_Math::Cap(_stacks, 3, INT_MAX);
	target_radius = radius = RE_Math::Cap(_radius, min_r, max_r);
	canChange = false;

	if(primID != -1) RE_SCENE->primitives->UnUsePrimitive(type, primID);
	int curves[2] = { 1, static_cast<int>(radius * 100.f) };
	primID = slices * stacks * curves[type == RE_Component::Type::TORUS || type == RE_Component::Type::TREFOILKNOT];

	auto md = RE_SCENE->primitives->GetPrimitiveMeshData(this, primID);
	VAO = md.first;
	triangle_count = md.second;
}

void RE_CompParametric::CopySetUp(GameObjectsPool* pool, RE_Component* _copy, const GO_UID parent)
{
	pool_gos = pool;
	if (useParent = (go = parent)) pool_gos->AtPtr(go)->ReportComponent(id, type);

	RE_CompParametric* copy = dynamic_cast<RE_CompParametric*>(_copy);
	color = copy->color;
	show_checkers = copy->show_checkers;
	min_r = copy->min_r;
	max_r = copy->max_r;
	ParametricSetUp(copy->slices, copy->stacks, copy->radius);
}

void RE_CompParametric::Draw() const
{
	unsigned int shader = dynamic_cast<RE_Shader*>(RE_RES->At(RE_InternalResources::GetDefaultShader()))->GetID();
	RE_GLCache::ChangeShader(shader);
	RE_ShaderImporter::setFloat4x4(shader, "model", GetGOCPtr()->GetTransformPtr()->GetGlobalMatrixPtr());

	if (!show_checkers) // Apply Diffuse Color
	{
		RE_ShaderImporter::setFloat(shader, "useColor", 1.0f);
		RE_ShaderImporter::setFloat(shader, "useTexture", 0.0f);
		RE_ShaderImporter::setFloat(shader, "cdiffuse", color);
		RE_ShaderImporter::setFloat(shader, "cspecular", math::vec::zero);
		RE_ShaderImporter::setFloat(shader, "shininess", 1.0f);
		RE_ShaderImporter::setFloat(shader, "opacity", 1.0f);
	}
	else // Apply Checkers Texture
	{
		glActiveTexture(GL_TEXTURE0);
		RE_ShaderImporter::setFloat(shader, "useColor", 0.0f);
		RE_ShaderImporter::setFloat(shader, "useTexture", 1.0f);
		RE_ShaderImporter::setUnsignedInt(shader, "tdiffuse", 0);
		RE_GLCache::ChangeTextureBind(RE_InternalResources::GetTextureChecker());
	}

	RE_GLCache::ChangeVAO(VAO);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(triangle_count) * 3, GL_UNSIGNED_SHORT, 0);
	RE_GLCache::ChangeVAO(0);
	RE_GLCache::ChangeTextureBind(0);
}

void RE_CompParametric::DrawProperties()
{
	if (ImGui::CollapsingHeader((name + " Primitive").c_str()))
	{
		ImGui::Checkbox("Use checkers texture", &show_checkers);
		ImGui::ColorEdit3("Diffuse Color", &color[0]);
		ImGui::PushItemWidth(75.0f);
		if (ImGui::DragInt("Slices", &target_slices, 1.0f, 3) && target_slices != slices) canChange = true;
		if (ImGui::DragInt("Stacks", &target_stacks, 1.0f, 3) && target_stacks != stacks) canChange = true;
		if ((type == RE_Component::Type::TORUS || type == RE_Component::Type::TREFOILKNOT) && ImGui::DragFloat("Radius", &target_radius, 0.1f, min_r, max_r) && target_radius != radius) canChange = true;

		ImGui::PopItemWidth();
		if (canChange && ImGui::Button("Apply"))
			ParametricSetUp(target_slices, target_stacks, target_radius);

		if (type == RE_Component::Type::PLANE && ImGui::Button("Convert To Mesh"))
			RE_INPUT->Push(RE_EventType::PLANE_CHANGE_TO_MESH, RE_SCENE, go);
	}
}

bool RE_CompParametric::DrawPrimPropierties()
{
	bool ret = false;
	if (ImGui::DragInt("Slices", &target_slices, 1.0f, 3) && target_slices != slices) ret = canChange = true;
	if (ImGui::DragInt("Stacks", &target_stacks, 1.0f, 3) && target_stacks != stacks) ret = canChange = true;
	if ((type == RE_Component::Type::TORUS || type == RE_Component::Type::TREFOILKNOT) && ImGui::DragFloat("Radius", &target_radius, 0.1f, min_r, max_r) && target_radius != radius) ret = canChange = true;

	if (canChange && ImGui::Button("Apply"))
		ParametricSetUp(target_slices, target_stacks, target_radius);

	return ret;
}

size_t RE_CompParametric::GetBinarySize() const
{
	return sizeof(float) * 4 + sizeof(int) * 2;
}

void RE_CompParametric::SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources) const
{
	node->PushFloatVector("color", color);
	node->Push("slices", slices);
	node->Push("stacks", stacks);
	node->Push("radius", radius);
}

void RE_CompParametric::DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources)
{
	color = node->PullFloatVector("color", color);
	ParametricSetUp(node->PullInt("slices", slices), node->PullInt("stacks", stacks), node->PullFloat("radius", radius));
}

void RE_CompParametric::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const
{
	size_t size = sizeof(float) * 3;
	memcpy(cursor, color.ptr(), size);
	cursor += size;

	size = sizeof(int);
	memcpy(cursor, &slices, size);
	cursor += size;

	size = sizeof(int);
	memcpy(cursor, &stacks, size);
	cursor += size;

	size = sizeof(float);
	memcpy(cursor, &radius, size);
	cursor += size;
}

void RE_CompParametric::DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources)
{
	size_t size = sizeof(float) * 3;
	memcpy(&RE_CompPrimitive::color[0], cursor, size);
	cursor += size;

	size = sizeof(int);
	memcpy(&slices, cursor, size);
	cursor += size;

	size = sizeof(int);
	memcpy(&stacks, cursor, size);
	cursor += size;

	size = sizeof(float);
	memcpy(&radius, cursor, size);
	cursor += size;

	ParametricSetUp(slices, stacks, radius);
}

size_t RE_CompParametric::GetParticleBinarySize() const
{
	return sizeof(int) * 2 + sizeof(float);
}

void RE_CompParametric::SerializeParticleJson(RE_Json* node) const
{
	node->Push("slices", slices);
	node->Push("stacks", stacks);
	node->Push("radius", radius);
	DEL(node)
}

void RE_CompParametric::DeserializeParticleJson(RE_Json* node)
{
	ParametricSetUp(node->PullInt("slices", slices), node->PullInt("stacks", stacks), node->PullFloat("radius", radius));
	DEL(node)
}

void RE_CompParametric::SerializeParticleBinary(char*& cursor) const
{
	size_t size = sizeof(int);
	memcpy(cursor, &slices, size);
	cursor += size;

	size = sizeof(int);
	memcpy(cursor, &stacks, size);
	cursor += size;

	size = sizeof(float);
	memcpy(cursor, &radius, size);
	cursor += size;
}

void RE_CompParametric::DeserializeParticleBinary(char*& cursor)
{
	size_t size = sizeof(int);
	memcpy(&slices, cursor, size);
	cursor += size;

	size = sizeof(int);
	memcpy(&stacks, cursor, size);
	cursor += size;

	size = sizeof(float);
	memcpy(&radius, cursor, size);
	cursor += size;

	ParametricSetUp(slices, stacks, radius);
}

///////   Plane   ////////////////////////////////////////////

const char* RE_CompPlane::TransformAsMeshResource()
{
	par_shapes_mesh* plane = par_shapes_create_plane(slices, stacks);

	size_t vertex_count = plane->npoints;

	float* points = new float[vertex_count * 3];
	float* normals = new float[vertex_count * 3];
	float* texCoords = new float[vertex_count * 2];

	size_t size = vertex_count * 3 * sizeof(float);
	memcpy(points, plane->points, size);
	memcpy(normals, plane->normals, size);
	size = vertex_count * 2 * sizeof(float);
	memcpy(texCoords, plane->tcoords, size);

	eastl::vector<unsigned int> index;
	for (int i = 0; i < plane->ntriangles * 3; i++)
		index.push_back(plane->triangles[i]);

	uint* indexA = new uint[static_cast<size_t>(plane->ntriangles) * 3];
	memcpy(indexA, &index[0], index.size() * sizeof(uint));

	bool exists = false;
	RE_Mesh* newMesh = new RE_Mesh();
	newMesh->SetVerticesAndIndex(points, indexA, vertex_count, plane->ntriangles, texCoords, normals);

	const char* meshMD5 = newMesh->CheckAndSave(&exists);
	if (!exists)
	{
		newMesh->SetName(eastl::string("Plane " + eastl::to_string(plane->ntriangles) + " triangles").c_str());
		newMesh->SetType(ResourceContainer::Type::MESH);
		RE_RES->Reference(newMesh);
	}
	else DEL(newMesh)

	par_shapes_free_mesh(plane);
	return meshMD5;
}