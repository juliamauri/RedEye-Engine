#include "RE_CompWater.h"

#include "RE_Json.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "RE_ECS_Pool.h"
#include "RE_GLCacheManager.h"
#include "RE_ResourceManager.h"
#include "RE_ShaderImporter.h"
#include "RE_Shader.h"

#include "MathGeoLib/include/Math/float4.h"
#include "par_shapes.h"
#include "ImGui/imgui.h"

RE_CompWater::RE_CompWater() : RE_Component(C_WATER) { box.SetFromCenterAndSize(math::vec::zero, math::vec::one); }

void RE_CompWater::CopySetUp(GameObjectsPool* pool, RE_Component* copy, const UID parent)
{
	pool_gos = pool;
	if (go = parent) pool_gos->AtPtr(go)->ReportComponent(id, C_WATER);

	RE_CompWater* cmpWater = dynamic_cast<RE_CompWater*>(copy);
	target_slices = slices = cmpWater->slices;
	target_stacks = stacks = cmpWater->stacks;

	waveLenght.second = cmpWater->waveLenght.second;
	amplitude.second = cmpWater->amplitude.second;
	speed.second = cmpWater->speed.second;
	is_linear.second = cmpWater->is_linear.second;
	direction.second = cmpWater->direction.second;
	center.second = cmpWater->center.second;
	steepness.second = cmpWater->steepness.second;
	numWaves.second = cmpWater->numWaves.second;
	cdiffuse.second = cmpWater->cdiffuse.second;
	shininess.second = cmpWater->shininess.second;
	foamMin.second = cmpWater->foamMin.second;
	foamMax.second = cmpWater->foamMax.second;
	foam_color.second = cmpWater->foam_color.second;
	opacity.second = cmpWater->opacity.second;
	distanceFoam.second = cmpWater->distanceFoam.second;

	if (cmpWater->VAO) {
		SetUpWaterUniforms();
		VAO = cmpWater->VAO;
		VBO = cmpWater->VBO;
		EBO = cmpWater->EBO;
		triangle_count = cmpWater->triangle_count;
	}
}

void RE_CompWater::Draw() const
{
	unsigned int textureCounter = 0;

	RE_Shader* shader = dynamic_cast<RE_Shader * >(App::resources->At(RE_ResourceManager::internalResources.GetDefaultWaterShader()));
	unsigned int shaderID = shader->GetID();
	RE_GLCacheManager::ChangeShader(shaderID);
	shader->UploadModel(GetGOCPtr()->GetTransformPtr()->GetGlobalMatrixPtr());
	
	LightMode lMode = ModuleRenderer3D::GetLightMode();
	
	glActiveTexture(GL_TEXTURE0 + textureCounter);
	RE_GLCacheManager::ChangeTextureBind(App::renderer3d->GetDepthTexture());
	shader->UploadDepth(textureCounter++);

	RE_GLCacheManager::ChangeShader(shaderID);
	switch (lMode)
	{
	case LIGHT_DISABLED:
	case LIGHT_GL:
	case LIGHT_DIRECT:
		for (uint i = 0; i < waterUniforms.size(); i++)
		{
			switch (waterUniforms[i].GetType())
			{
			case RE_Cvar::BOOL: RE_ShaderImporter::setBool(waterUniforms[i].location, waterUniforms[i].AsBool()); break;
			case RE_Cvar::INT: RE_ShaderImporter::setInt(waterUniforms[i].location, waterUniforms[i].AsInt()); break;
			case RE_Cvar::FLOAT: RE_ShaderImporter::setFloat(waterUniforms[i].location, waterUniforms[i].AsFloat()); break;
			case RE_Cvar::FLOAT2:
			{
				const float* f_ptr = waterUniforms[i].AsFloatPointer();
				RE_ShaderImporter::setFloat(waterUniforms[i].location, f_ptr[0], f_ptr[1]);
				break;
			}
			case RE_Cvar::FLOAT3:
			{
				const float* f_ptr = waterUniforms[i].AsFloatPointer();
				RE_ShaderImporter::setFloat(waterUniforms[i].location, f_ptr[0], f_ptr[1], f_ptr[2]);
				break;
			}
			case RE_Cvar::SAMPLER: //Only one case
				glActiveTexture(GL_TEXTURE0 + textureCounter);
				RE_GLCacheManager::ChangeTextureBind(waterFoam.second);
				RE_ShaderImporter::setUnsignedInt(waterUniforms[i].location, textureCounter++);
				break;
			}
		}
		break;
	case LIGHT_DEFERRED:
		for (uint i = 0; i < waterUniforms.size(); i++)
		{
			switch (waterUniforms[i].GetType())
			{
			case RE_Cvar::BOOL: RE_ShaderImporter::setBool(waterUniforms[i].locationDeferred, waterUniforms[i].AsBool()); break;
			case RE_Cvar::INT: RE_ShaderImporter::setInt(waterUniforms[i].locationDeferred, waterUniforms[i].AsInt()); break;
			case RE_Cvar::FLOAT: RE_ShaderImporter::setFloat(waterUniforms[i].locationDeferred, waterUniforms[i].AsFloat()); break;
			case RE_Cvar::FLOAT2:
			{
				const float* f_ptr = waterUniforms[i].AsFloatPointer();
				RE_ShaderImporter::setFloat(waterUniforms[i].locationDeferred, f_ptr[0], f_ptr[1]);
				break;
			}
			case RE_Cvar::FLOAT3:
			{
				const float* f_ptr = waterUniforms[i].AsFloatPointer();
				RE_ShaderImporter::setFloat(waterUniforms[i].locationDeferred, f_ptr[0], f_ptr[1], f_ptr[2]);
				break;
			}
			{
			case RE_Cvar::SAMPLER: //Only one case
				glActiveTexture(GL_TEXTURE0 + textureCounter);
				RE_GLCacheManager::ChangeTextureBind(waterFoam.second);
				RE_ShaderImporter::setUnsignedInt(waterUniforms[i].locationDeferred, textureCounter++);
				break;
			}
			}
		}

		break;
	}

	RE_GLCacheManager::ChangeVAO(VAO);
	glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_INT, nullptr);
	RE_GLCacheManager::ChangeVAO(0);
	RE_GLCacheManager::ChangeShader(0);
}

void RE_CompWater::DrawProperties()
{
	if (ImGui::CollapsingHeader("Water", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Plane");

		static bool canChange = false;
		if (ImGui::DragInt("Slices", &target_slices, 1.0f, 3) && target_slices != slices) canChange = true;
		if (ImGui::DragInt("Stacks", &target_stacks, 1.0f, 3) && target_stacks != stacks) canChange = true;
		
		if (canChange && ImGui::Button("Apply"))
		{
			slices = target_slices;
			stacks = target_stacks;
			GeneratePlane();
			canChange = false;
		}

		ImGui::Separator();

		ImGui::Text("Wave Vertex");

		if (ImGui::DragFloat("Wave Lenght", &waveLenght.second, 0.1f, 0.0f)) 
			waveLenght.first->SetValue(waveLenght.second);

		if (ImGui::DragFloat("Amplitude", &amplitude.second, 0.1f, 0.0f))
			amplitude.first->SetValue(amplitude.second);

		if (ImGui::DragFloat("Speed", &speed.second, 0.1f, 0.0f))
			speed.first->SetValue(speed.second);

		if (ImGui::Checkbox("Linear", &is_linear.second))
			is_linear.first->SetValue(is_linear.second);

		if (ImGui::DragFloat2("Direction", direction.second.ptr(), 0.1f))
			direction.first->SetValue(direction.second);

		if (ImGui::DragFloat2("Center", center.second.ptr(), 0.1f))
			center.first->SetValue(center.second);

		if (ImGui::DragFloat("Steepness", &steepness.second, 0.1f, 0.0f))
			steepness.first->SetValue(steepness.second);

		if (ImGui::DragInt("Num Waves", &numWaves.second, 1.0f, 1))
			numWaves.first->SetValue(numWaves.second);

		ImGui::Separator();

		ImGui::Text("Wave Fragment");

		if(ImGui::ColorEdit3("Color", cdiffuse.second.ptr()))
			cdiffuse.first->SetValue(cdiffuse.second);

		if (ImGui::DragFloat("Shininess", &shininess.second, 0.1f, 1.0f, 32.f))
			shininess.first->SetValue(shininess.second);

		if (ImGui::DragFloat("Foam Min", &foamMin.second, 0.01f, 0.0f, 1.f))
			foamMin.first->SetValue(foamMin.second);

		if (ImGui::DragFloat("Foam Max", &foamMax.second, 0.01f, 0.0f, 1.f))
			foamMax.first->SetValue(foamMax.second);

		if (ImGui::ColorEdit3("Foam Color", foam_color.second.ptr()))
			foam_color.first->SetValue(foam_color.second);

		if (ImGui::DragFloat("Opacity", &opacity.second, 0.01f, 0.0f, 1.f))
			opacity.first->SetValue(opacity.second);
		
		if (ImGui::DragFloat("Distance Dynamic Foam", &distanceFoam.second, 0.00001f, 0.0f, 0.0005f, "%.5f" ))
			distanceFoam.first->SetValue(distanceFoam.second);
	}
}

void RE_CompWater::SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources) const
{
	node->PushInt("slices", slices);
	node->PushInt("stacks", stacks);
	node->PushFloat("waveLenght", waveLenght.second);
	node->PushFloat("amplitude", amplitude.second);
	node->PushFloat("speed", speed.second);
	node->PushBool("is_linear", is_linear.second);
	node->PushFloat4("dirCe", { direction.second.x, direction.second.y, center.second.x, center.second.y });
	node->PushFloat("steepness", steepness.second);
	node->PushInt("numWaves", numWaves.second);
	node->PushFloatVector("cdiffuse", cdiffuse.second);
	node->PushFloat("shininess", shininess.second);
	node->PushFloat("foamMin", foamMin.second);
	node->PushFloat("foamMax", foamMax.second);
	node->PushFloatVector("foamColor", foam_color.second);
	node->PushFloat("opacity", opacity.second);
	node->PushFloat("distanceFoam", distanceFoam.second);
}

void RE_CompWater::DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources)
{
	target_slices = slices = node->PullInt("slices", slices);
	target_stacks = stacks = node->PullInt("stacks", stacks);
	waveLenght.second = node->PullFloat("waveLenght", waveLenght.second);
	amplitude.second = node->PullFloat("amplitude", amplitude.second);
	speed.second = node->PullFloat("speed", speed.second);
	is_linear.second = node->PullBool("is_linear", is_linear.second);

	math::float4 dirCe(node->PullFloat4("dirCe", { direction.second.x, direction.second.y, center.second.x, center.second.y }));
	direction.second = dirCe.xy();
	center.second.Set(dirCe.z, dirCe.w);

	steepness.second = node->PullFloat("steepness", steepness.second);
	numWaves.second = node->PullInt("numWaves", numWaves.second);
	cdiffuse.second = node->PullFloatVector("cdiffuse", cdiffuse.second);
	shininess.second = node->PullFloat("shininess", shininess.second);
	foamMin.second = node->PullFloat("foamMin", foamMin.second);
	foamMax.second = node->PullFloat("foamMax", foamMax.second);
	foam_color.second = node->PullFloatVector("foamColor", foam_color.second);
	opacity.second = node->PullFloat("opacity", opacity.second);
	distanceFoam.second = node->PullFloat("distanceFoam", distanceFoam.second);
}

unsigned int RE_CompWater::GetBinarySize() const
{
	return sizeof(int) * 3 + sizeof(float) * 19 + sizeof(bool);
}

void RE_CompWater::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const
{
	size_t size = sizeof(int);
	memcpy(cursor, &slices, size); cursor += size;
	memcpy(cursor, &stacks, size); cursor += size;
	size = sizeof(float);
	memcpy(cursor, &waveLenght.second, size); cursor += size;
	memcpy(cursor, &amplitude.second, size); cursor += size;
	memcpy(cursor, &speed.second, size); cursor += size;
	size = sizeof(bool);
	memcpy(cursor, &is_linear.second, size); cursor += size;
	size = sizeof(float) * 2;
	memcpy(cursor, direction.second.ptr(), size); cursor += size;
	memcpy(cursor, center.second.ptr(), size); cursor += size;
	size = sizeof(float);
	memcpy(cursor, &steepness.second, size); cursor += size;
	size = sizeof(int);
	memcpy(cursor, &numWaves.second, size); cursor += size;
	size = sizeof(float) * 3;
	memcpy(cursor, &cdiffuse.second, size); cursor += size;
	size = sizeof(float);
	memcpy(cursor, &shininess.second, size); cursor += size;
	memcpy(cursor, &foamMin.second, size); cursor += size;
	memcpy(cursor, &foamMax.second, size); cursor += size;
	size = sizeof(float) * 3;
	memcpy(cursor, &foam_color.second, size); cursor += size;
	size = sizeof(float);
	memcpy(cursor, &opacity.second, size); cursor += size;
	memcpy(cursor, &distanceFoam.second, size); cursor += size;
}

void RE_CompWater::DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources)
{
	size_t size = sizeof(int);
	memcpy(&slices, cursor, size); cursor += size;
	memcpy(&stacks, cursor, size); cursor += size;
	target_slices = slices;
	target_stacks = stacks;
	size = sizeof(float);
	memcpy(&waveLenght.second, cursor, size); cursor += size;
	memcpy(&amplitude.second, cursor, size); cursor += size;
	memcpy(&speed.second, cursor, size); cursor += size;
	size = sizeof(bool);
	memcpy(&is_linear.second, cursor, size); cursor += size;
	size = sizeof(float) * 2;
	memcpy(direction.second.ptr(), cursor, size); cursor += size;
	memcpy(center.second.ptr(), cursor, size); cursor += size;
	size = sizeof(float);
	memcpy(&steepness.second, cursor, size); cursor += size;
	size = sizeof(int);
	memcpy(&numWaves.second, cursor, size); cursor += size;
	size = sizeof(float) * 3;
	memcpy(&cdiffuse.second, cursor, size); cursor += size;
	size = sizeof(float);
	memcpy(&shininess.second, cursor, size); cursor += size;
	memcpy(&foamMin.second, cursor, size); cursor += size;
	memcpy(&foamMax.second, cursor, size); cursor += size;
	size = sizeof(float) * 3;
	memcpy(&foam_color.second, cursor, size); cursor += size;
	size = sizeof(float);
	memcpy(&opacity.second, cursor, size); cursor += size;
	memcpy(&distanceFoam.second, cursor, size); cursor += size;
}

math::AABB RE_CompWater::GetAABB() const
{
	return box;
}

bool RE_CompWater::CheckFaceCollision(const math::Ray& local_ray, float& distance) const
{
	// TODO Rub: WaterPlane raycast checking (only check on aabb)
	return false;
}

void RE_CompWater::UseResources()
{
	GeneratePlane();
	SetUpWaterUniforms();
}

void RE_CompWater::UnUseResources()
{
	waveLenght.first = nullptr;
	amplitude.first = nullptr;
	speed.first = nullptr;
	is_linear.first = nullptr;
	direction.first = nullptr;
	center.first = nullptr;
	steepness.first = nullptr;
	numWaves.first = nullptr;
	cdiffuse.first = nullptr;
	shininess.first = nullptr;
	foamMin.first = nullptr;
	foamMax.first = nullptr;
	foam_color.first = nullptr;
	opacity.first = nullptr;
	distanceFoam.first = nullptr;
	waterFoam.first = nullptr;

	waterUniforms.clear();

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

unsigned int RE_CompWater::GetVAO() const { return VAO; }
unsigned int RE_CompWater::GetTriangles() const { return triangle_count; }

void RE_CompWater::GeneratePlane()
{
	par_shapes_mesh* plane = par_shapes_create_plane(slices, stacks);

	float* points = new float[plane->npoints * 3];
	float* normals = new float[plane->npoints * 3];
	float* texCoords = new float[plane->npoints * 2];

	uint meshSize = 0;
	uint stride = 0;

	size_t size = plane->npoints * 3 * sizeof(float);
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

	triangle_count = plane->ntriangles;

	eastl::vector<unsigned int> index;
	for (unsigned int i = 0; i < triangle_count * 3; i++)
		index.push_back(plane->triangles[i]);

	uint* indexA = new uint[triangle_count * 3];
	memcpy(indexA, &index[0], index.size() * sizeof(uint));

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

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	RE_GLCacheManager::ChangeVAO(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	stride *= sizeof(float);

	float* meshBuffer = new float[meshSize];
	float* cursor = meshBuffer;

	for (uint i = 0; i < static_cast<uint>(plane->npoints); i++)
	{
		uint indexArray = i * 3;
		uint indexTexCoord = i * 2;

		size_t size = 3;
		memcpy(cursor, &points[indexArray], size * sizeof(float));
		cursor += size;
		memcpy(cursor, &normals[indexArray], size * sizeof(float));
		cursor += size;
		size = 2;
		memcpy(cursor, &texCoords[indexTexCoord], size * sizeof(float));
		cursor += size;
	}

	glBufferData(GL_ARRAY_BUFFER, meshSize * sizeof(float), &meshBuffer[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangle_count * 3 * sizeof(unsigned int), indexA, GL_STATIC_DRAW);

	// vertex positions
	int accumulativeOffset = 0;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(accumulativeOffset));
	accumulativeOffset += sizeof(float) * 3;

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(accumulativeOffset));
	accumulativeOffset += sizeof(float) * 3;

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(accumulativeOffset));

	RE_GLCacheManager::ChangeVAO(0);

	DEL_A(points);
	DEL_A(normals);
	DEL_A(texCoords);
	DEL_A(indexA);
	DEL_A(meshBuffer);
}

void RE_CompWater::SetUpWaterUniforms()
{
	waterUniforms = RE_ResourceManager::internalResources.GetWaterUniforms();

	for (unsigned int i = 0; i < waterUniforms.size(); i++) {
		if (waterUniforms[i].name == "waveLength") {
			waveLenght.first = &waterUniforms[i];
			waterUniforms[i].SetValue(waveLenght.second);
		}
		else if (waterUniforms[i].name == "amplitude") {
			amplitude.first = &waterUniforms[i];
			waterUniforms[i].SetValue(amplitude.second);
			box.SetFromCenterAndSize(math::vec::one * 0.5f, { 1.0f, amplitude.second, 1.0f });
		}
		else if (waterUniforms[i].name == "speed") {
			speed.first = &waterUniforms[i];
			waterUniforms[i].SetValue(speed.second);
		}
		else if (waterUniforms[i].name == "is_linear") {
			is_linear.first = &waterUniforms[i];
			waterUniforms[i].SetValue(is_linear.second);
		}
		else if (waterUniforms[i].name == "direction") {
			direction.first = &waterUniforms[i];
			waterUniforms[i].SetValue(direction.second);
		}
		else if (waterUniforms[i].name == "center") {
			center.first = &waterUniforms[i];
			waterUniforms[i].SetValue(center.second);
		}
		else if (waterUniforms[i].name == "steepness") {
			steepness.first = &waterUniforms[i];
			waterUniforms[i].SetValue(steepness.second);
		}
		else if (waterUniforms[i].name == "numWaves") {
			numWaves.first = &waterUniforms[i];
			waterUniforms[i].SetValue(numWaves.second);
		}
		else if (waterUniforms[i].name == "cdiffuse") {
			cdiffuse.first = &waterUniforms[i];
			waterUniforms[i].SetValue(cdiffuse.second);
		}
		else if (waterUniforms[i].name == "shininess") {
			shininess.first = &waterUniforms[i];
			waterUniforms[i].SetValue(shininess.second);
		}
		else if (waterUniforms[i].name == "foamMin") {
			foamMin.first = &waterUniforms[i];
			waterUniforms[i].SetValue(foamMin.second);
		}
		else if (waterUniforms[i].name == "foamMax") {
			foamMax.first = &waterUniforms[i];
			waterUniforms[i].SetValue(foamMax.second);
		}
		else if (waterUniforms[i].name == "foam_color") {
			foam_color.first = &waterUniforms[i];
			waterUniforms[i].SetValue(foam_color.second);
		}
		else if (waterUniforms[i].name == "opacity") {
			opacity.first = &waterUniforms[i];
			waterUniforms[i].SetValue(opacity.second);
		}
		else if (waterUniforms[i].name == "distanceFoam") {
			distanceFoam.first = &waterUniforms[i];
			waterUniforms[i].SetValue(distanceFoam.second);
		}
		else if (waterUniforms[i].name == "water_foam") {
			waterFoam.first = &waterUniforms[i];
			waterFoam.second = RE_ResourceManager::internalResources.GetTextureWaterFoam();
		}
	}
}
