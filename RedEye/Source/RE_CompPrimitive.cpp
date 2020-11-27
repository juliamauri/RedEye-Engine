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
#include "RE_GLCacheManager.h"
#include "RE_Mesh.h"
#include "RE_Shader.h"

#ifndef PAR_SHAPES_IMPLEMENTATION
#define PAR_SHAPES_IMPLEMENTATION
#endif

#include "par_shapes.h"

RE_CompPrimitive::RE_CompPrimitive(ComponentType t) : RE_Component(t)
{
	color = math::vec(1.0f, 0.15f, 0.15f);
}

bool RE_CompPrimitive::CheckFaceCollision(const math::Ray& ray, float& distance) const
{
	// TODO Rub: Primitive raycast checking
	return false;
}

void RE_CompPrimitive::DeleteBuffers()
{
	if (VAO)
	{
		glDeleteVertexArrays(1, static_cast<GLuint*>(&VAO));
		VAO = 0;
	}
	if (VBO)
	{
		glDeleteBuffers(1, static_cast<GLuint*>(&VBO));
		VBO = 0;
	}
	if (EBO)
	{
		glDeleteBuffers(1, static_cast<GLuint*>(&EBO));
		EBO = 0;
	}
}

void RE_CompPrimitive::SetColor(float r, float g, float b) { color.Set(r, g, b); }
void RE_CompPrimitive::SetColor(math::vec nColor) { color = nColor; }
unsigned int RE_CompPrimitive::GetVAO() const { return VAO; }
void RE_CompPrimitive::SetVAO(unsigned int vao) { VAO = vao; }
unsigned int RE_CompPrimitive::GetTriangleCount() const { return triangle_count; }

///////   Grid   ////////////////////////////////////////////
RE_CompGrid::~RE_CompGrid() { DeleteBuffers(); }

void RE_CompGrid::GridSetUp(int newD)
{
	if (!useParent && !transform) {
		transform = new RE_CompTransform();
		transform->SetParent(0ull);
	}

	if (newD < 0) newD = 10;
	
	if (VAO) DeleteBuffers();

	tmpSb = divisions = newD;

	eastl::vector<float> vertices;
	float d = static_cast<float>(divisions);
	float distance = d * 2.5f;
	float f = 0.f;
	for (; f < d; f += 0.5f)
	{
		vertices.push_back((f * 5.f) - distance);
		vertices.push_back(0.f);
		vertices.push_back(distance);
		vertices.push_back((f * 5.f) - distance);
		vertices.push_back(0.f);
		vertices.push_back(-distance);
		vertices.push_back(distance);
		vertices.push_back(0.f);
		vertices.push_back((f * 5.f) - distance);
		vertices.push_back(-distance);
		vertices.push_back(0.f);
		vertices.push_back((f * 5.f) - distance);
	}

	vertices.push_back((f * 5.f) - distance);
	vertices.push_back(0.f);
	vertices.push_back(distance);
	vertices.push_back((f * 5.f) - distance);
	vertices.push_back(0.f);
	vertices.push_back(-distance);
	vertices.push_back(distance);
	vertices.push_back(0.f);
	vertices.push_back((f * 5.f) - distance);
	vertices.push_back(-distance);
	vertices.push_back(0.f);
	vertices.push_back((f * 5.f) - distance);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	RE_GLCacheManager::ChangeVAO(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	RE_GLCacheManager::ChangeVAO(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void RE_CompGrid::CopySetUp(GameObjectsPool* pool, RE_Component* _copy, const UID parent)
{
	pool_gos = pool;
	if (useParent = (go = parent)) pool_gos->AtPtr(go)->ReportComponent(id, type);

	RE_CompGrid* copy = dynamic_cast<RE_CompGrid*>(_copy);
	color = copy->color;
	GridSetUp(copy->divisions);
}

void RE_CompGrid::Draw() const
{
	unsigned int shader = dynamic_cast<RE_Shader*>(App::resources->At(App::internalResources.GetDefaultShader()))->GetID();
	RE_GLCacheManager::ChangeShader(shader);
	RE_ShaderImporter::setFloat4x4(shader, "model", GetTransformPtr()->GetGlobalMatrixPtr());
	RE_ShaderImporter::setFloat(shader, "useColor", 1.0f);
	RE_ShaderImporter::setFloat(shader, "useTexture", 0.0f);
	RE_ShaderImporter::setFloat(shader, "cdiffuse", color);

	RE_GLCacheManager::ChangeVAO(VAO);
	glDrawArrays(GL_LINES, 0, (divisions * 8) + 4);
	RE_GLCacheManager::ChangeVAO(0);
}

void RE_CompGrid::DrawProperties()
{
	// TODO Rub: editor properties
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

unsigned int RE_CompGrid::GetBinarySize() const
{
	return sizeof(float) * 3u + sizeof(int);
}

void RE_CompGrid::SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources) const
{
	node->PushFloatVector("color", color);
	node->PushInt("divisions", divisions);
}

void RE_CompGrid::DeserializeJson(JSONNode* node, eastl::map<int, const char*>* resources)
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
RE_CompRock::~RE_CompRock() { DeleteBuffers(); }

void RE_CompRock::RockSetUp(int _seed, int _subdivions)
{
	GenerateNewRock(_seed, RE_Math::Cap(_subdivions, 1, 5));
}

void RE_CompRock::CopySetUp(GameObjectsPool* pool, RE_Component* _copy, const UID parent)
{
	pool_gos = pool;
	if (useParent = (go = parent)) pool_gos->AtPtr(go)->ReportComponent(id, type);

	RE_CompRock* copy = dynamic_cast<RE_CompRock*>(_copy);
	color = copy->color;
	RockSetUp(copy->seed, copy->nsubdivisions);
}

void RE_CompRock::Draw() const
{
	unsigned int shader = dynamic_cast<RE_Shader*>(App::resources->At(App::internalResources.GetDefaultShader()))->GetID();
	RE_GLCacheManager::ChangeShader(shader);
	RE_ShaderImporter::setFloat4x4(shader, "model", GetGOCPtr()->GetTransformPtr()->GetGlobalMatrixPtr());

	// Apply Diffuse Color
	RE_ShaderImporter::setFloat(shader, "useColor", 1.0f);
	RE_ShaderImporter::setFloat(shader, "useTexture", 0.0f);
	RE_ShaderImporter::setFloat(shader, "cdiffuse", color);

	// Draw
	RE_GLCacheManager::ChangeVAO(VAO);
	glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_SHORT, 0);
	RE_GLCacheManager::ChangeVAO(0);
}

void RE_CompRock::DrawProperties()
{
	if (ImGui::CollapsingHeader("Rock Primitive"))
	{
		int tmpSe = seed, tmpSb = nsubdivisions;

		ImGui::Text("Can't checker because don't support Texture Coords");
		ImGui::ColorEdit3("Diffuse Color", &RE_CompPrimitive::color[0]);
		ImGui::PushItemWidth(75.0f);

		if (ImGui::DragInt("Seed", &tmpSe, 1.0f) && seed != tmpSe)
		{
			seed = tmpSe;
			canChange = true;
		}

		if (ImGui::DragInt("Num Subdivisions", &tmpSb, 1.0f, 1, 5))
		{
			if (tmpSb >= 3 && nsubdivisions != tmpSb)
			{
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

void RE_CompRock::SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources) const
{
	node->PushFloatVector("color", color);
	node->PushInt("seed", seed);
	node->PushInt("nsubdivisions", nsubdivisions);
}

void RE_CompRock::DeserializeJson(JSONNode* node, eastl::map<int, const char*>* resources)
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

void RE_CompRock::GenerateNewRock(int seed, int subdivisions)
{
	if (canChange)
	{
		if (VAO != 0)
		{
			glDeleteVertexArrays(1, &(GLuint)VAO);
			glDeleteBuffers(1, &(GLuint)VBO);
			glDeleteBuffers(1, &(GLuint)EBO);
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
		for (int i = 0; i < rock->npoints; i++)
		{
			uint cursorSize = 3;
			size_t size = sizeof(float) * 3;

			memcpy(cursor, &points[i * 3], size);
			cursor += cursorSize;

			memcpy(cursor, &normals[i * 3], size);
			cursor += cursorSize;
		}

		glGenVertexArrays(1, &VAO);
		RE_GLCacheManager::ChangeVAO(VAO);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, meshSize * sizeof(float), meshBuffer, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, NULL);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));

		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, rock->ntriangles * sizeof(unsigned short) * 3, rock->triangles, GL_STATIC_DRAW);

		RE_GLCacheManager::ChangeVAO(0);

		triangle_count = rock->ntriangles;

		par_shapes_free_mesh(rock);
		DEL_A(points);
		DEL_A(normals);
		DEL_A(meshBuffer);

		canChange = false;
	}
}

///////   Platonic   ////////////////////////////////////////////
RE_CompPlatonic::~RE_CompPlatonic() { DeleteBuffers(); }

void RE_CompPlatonic::PlatonicSetUp(unsigned int vao, unsigned int t_count)
{
	if (vao)
	{
		VAO = vao;
		triangle_count = t_count;
	}
	else
	{
		eastl::pair<unsigned int, unsigned int> data = App::primitives.GetPlatonicData(type);
		VAO = data.first;
		triangle_count = data.second;
	}
}

void RE_CompPlatonic::CopySetUp(GameObjectsPool* pool, RE_Component* _copy, const UID parent)
{
	pool_gos = pool;
	if (useParent = (go = parent)) pool_gos->AtPtr(go)->ReportComponent(id, type);

	RE_CompPlatonic* copy = dynamic_cast<RE_CompPlatonic*>(_copy);
	color = copy->color;

	PlatonicSetUp(copy->VAO, copy->triangle_count);
}

void RE_CompPlatonic::Draw() const
{
	unsigned int shader = dynamic_cast<RE_Shader*>(App::resources->At(App::internalResources.GetDefaultShader()))->GetID();
	RE_GLCacheManager::ChangeShader(shader);
	RE_ShaderImporter::setFloat4x4(shader, "model", GetGOCPtr()->GetTransformPtr()->GetGlobalMatrixPtr());

	// Apply Diffuse Color
	RE_ShaderImporter::setFloat(shader, "useColor", 1.0f);
	RE_ShaderImporter::setFloat(shader, "useTexture", 0.0f);
	RE_ShaderImporter::setFloat(shader, "cdiffuse", color);

	// Draw
	RE_GLCacheManager::ChangeVAO(VAO);
	glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_SHORT, 0);
	RE_GLCacheManager::ChangeVAO(0);
}

void RE_CompPlatonic::DrawProperties()
{
	if (ImGui::CollapsingHeader((pName + " Primitive").c_str()))
	{
		ImGui::Text("Can't checker because don't support Texture Coords");
		ImGui::ColorEdit3("Diffuse Color", &color[0]);
	}
}

unsigned int RE_CompPlatonic::GetBinarySize() const
{
	return sizeof(float) * 3;
}

void RE_CompPlatonic::SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources) const
{
	node->PushFloatVector("color", color);
}

void RE_CompPlatonic::DeserializeJson(JSONNode* node, eastl::map<int, const char*>* resources)
{
	color = node->PullFloatVector("color", color);
	App::primitives.SetUpComponentPrimitive(this);
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

	App::primitives.SetUpComponentPrimitive(this);
}

///////   Parametric   ////////////////////////////////////////////
RE_CompParametric::RE_CompParametric(ComponentType t, const char* _name) : RE_CompPrimitive(t), name(_name) {}
RE_CompParametric::~RE_CompParametric() { DeleteBuffers(); }

void RE_CompParametric::ParametricSetUp(int _slices, int _stacks, float _radius)
{
	target_slices = slices = RE_Math::Cap(_slices, 1, 3);
	target_stacks = stacks = RE_Math::Cap(_stacks, 1, 3);
	target_radius = radius = _radius;
	canChange = false;
	GenerateParametric();
}

void RE_CompParametric::CopySetUp(GameObjectsPool* pool, RE_Component* _copy, const UID parent)
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
	unsigned int shader = dynamic_cast<RE_Shader*>(App::resources->At(App::internalResources.GetDefaultShader()))->GetID();
	RE_GLCacheManager::ChangeShader(shader);
	RE_ShaderImporter::setFloat4x4(shader, "model", GetGOCPtr()->GetTransformPtr()->GetGlobalMatrixPtr());

	if (!show_checkers) // Apply Diffuse Color
	{
		RE_ShaderImporter::setFloat(shader, "useColor", 1.0f);
		RE_ShaderImporter::setFloat(shader, "useTexture", 0.0f);
		RE_ShaderImporter::setFloat(shader, "cdiffuse", color);
	}
	else // Apply Checkers Texture
	{
		glActiveTexture(GL_TEXTURE0);
		RE_ShaderImporter::setFloat(shader, "useColor", 0.0f);
		RE_ShaderImporter::setFloat(shader, "useTexture", 1.0f);
		RE_ShaderImporter::setUnsignedInt(shader, "tdiffuse", 0);
		RE_GLCacheManager::ChangeTextureBind(App::internalResources.GetTextureChecker());
	}

	RE_GLCacheManager::ChangeVAO(VAO);
	glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_SHORT, 0);
	RE_GLCacheManager::ChangeVAO(0);
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
		if (radius > 0.f && ImGui::DragFloat("Radius", &target_radius, 0.1f, min_r, max_r) && target_radius != radius) canChange = true;

		ImGui::PopItemWidth();
		if (canChange && ImGui::Button("Apply"))
		{
			GenerateParametric();
			canChange = false;
		}

		if (type == C_PLANE && ImGui::Button("Convert To Mesh")) 
			Event::Push(RE_EventType::PLANE_CHANGE_TO_MESH, App::scene, go);
	}
}

unsigned int RE_CompParametric::GetBinarySize() const
{
	return sizeof(float) * 4 + sizeof(int) * 2;
}

void RE_CompParametric::SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources) const
{
	node->PushFloatVector("color", color);
	node->PushInt("slices", slices);
	node->PushInt("stacks", stacks);
	node->PushFloat("radius", radius);
}

void RE_CompParametric::DeserializeJson(JSONNode* node, eastl::map<int, const char*>* resources)
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

void RE_CompParametric::UploadParametric(par_shapes_mesh_s* param)
{
	if (VAO != 0) DeleteBuffers();

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

	for (int i = 0; i < param->npoints; i++)
	{
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

	glGenVertexArrays(1, &VAO);
	RE_GLCacheManager::ChangeVAO(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, meshSize * sizeof(float), meshBuffer, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(sizeof(float) * 3u));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(sizeof(float) * 6u));

	glGenBuffers(1, &(GLuint)EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, param->ntriangles * sizeof(unsigned short) * 3u, param->triangles, GL_STATIC_DRAW);

	RE_GLCacheManager::ChangeVAO(0);

	triangle_count = param->ntriangles;

	DEL_A(points);
	DEL_A(normals);
	DEL_A(texCoords);
	DEL_A(meshBuffer);
}

///////   Plane   ////////////////////////////////////////////
RE_CompPlane::RE_CompPlane() : RE_CompParametric(C_PLANE, "Plane") {}
RE_CompPlane::~RE_CompPlane() {}

const char* RE_CompPlane::TransformAsMeshResource()
{
	par_shapes_mesh* plane = par_shapes_create_plane(slices, stacks);

	float* points = new float[plane->npoints * 3];
	float* normals = new float[plane->npoints * 3];
	float* texCoords = new float[plane->npoints * 2];

	size_t size = plane->npoints * 3 * sizeof(float);
	memcpy(points, plane->points, size);
	memcpy(normals, plane->normals, size);
	size = plane->npoints * 2 * sizeof(float);
	memcpy(texCoords, plane->tcoords, size);

	eastl::vector<unsigned int> index;
	for (int i = 0; i < plane->ntriangles * 3; i++)
		index.push_back(plane->triangles[i]);

	uint* indexA = new uint[plane->ntriangles * 3];
	memcpy(indexA, &index[0], index.size() * sizeof(uint));

	bool exists = false;
	RE_Mesh* newMesh = new RE_Mesh();
	newMesh->SetVerticesAndIndex(points, indexA, plane->npoints, plane->ntriangles, texCoords, normals);

	const char* meshMD5 = newMesh->CheckAndSave(&exists);
	if (!exists)
	{
		newMesh->SetName(eastl::string("Plane " + eastl::to_string(plane->ntriangles) + " triangles").c_str());
		newMesh->SetType(Resource_Type::R_MESH);
		App::resources->Reference(newMesh);
	}
	else DEL(newMesh);

	par_shapes_free_mesh(plane);
	return meshMD5;
}

void RE_CompPlane::GenerateParametric()
{
	par_shapes_mesh* plane = par_shapes_create_plane(slices, stacks);
	UploadParametric(plane);
	par_shapes_free_mesh(plane);
}

///////   Sphere   ////////////////////////////////////////////
RE_CompSphere::RE_CompSphere() : RE_CompParametric(C_SPHERE, "Sphere") {}
RE_CompSphere::~RE_CompSphere() {}

void RE_CompSphere::GenerateParametric()
{
	par_shapes_mesh* sphere = par_shapes_create_parametric_sphere(slices, stacks);
	UploadParametric(sphere);
	par_shapes_free_mesh(sphere);
}

///////   Cylinder   ////////////////////////////////////////////
RE_CompCylinder::RE_CompCylinder() : RE_CompParametric(C_CYLINDER, "Cylinder") {}
RE_CompCylinder::~RE_CompCylinder() {}

void RE_CompCylinder::GenerateParametric()
{
	par_shapes_mesh* cylinder = par_shapes_create_cylinder(slices, stacks);
	UploadParametric(cylinder);
	par_shapes_free_mesh(cylinder);
}

///////   HemiSphere   ////////////////////////////////////////////
RE_CompHemiSphere::RE_CompHemiSphere() : RE_CompParametric(C_HEMISHPERE, "HemiSphere") {}
RE_CompHemiSphere::~RE_CompHemiSphere() {}

void RE_CompHemiSphere::GenerateParametric()
{
	par_shapes_mesh* hemishpere = par_shapes_create_hemisphere(slices, stacks);
	UploadParametric(hemishpere);
	par_shapes_free_mesh(hemishpere);
}

///////   Torus   ////////////////////////////////////////////
RE_CompTorus::RE_CompTorus() : RE_CompParametric(C_TORUS, "Torus")
{
	min_r = 0.1f;
	max_r = 1.0f;
}
RE_CompTorus::~RE_CompTorus() {}

void RE_CompTorus::GenerateParametric()
{
	par_shapes_mesh* torus = par_shapes_create_torus(slices, stacks, radius);
	UploadParametric(torus);
	par_shapes_free_mesh(torus);
}

///////   TrefoiKnot   ////////////////////////////////////////////
RE_CompTrefoiKnot::RE_CompTrefoiKnot() : RE_CompParametric(C_TREFOILKNOT,"Trefoil Knot")
{
	min_r = 0.5f;
	max_r = 3.0f;
}
RE_CompTrefoiKnot::~RE_CompTrefoiKnot() {}

void RE_CompTrefoiKnot::GenerateParametric()
{
	par_shapes_mesh* trefoilknot = par_shapes_create_trefoil_knot(slices, stacks, radius);
	UploadParametric(trefoilknot);
	par_shapes_free_mesh(trefoilknot);
}