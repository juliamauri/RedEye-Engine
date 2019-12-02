#include "RE_CompMesh.h"

#include "Application.h"
#include "ModuleScene.h"

#include "RE_Math.h"
#include "RE_FileSystem.h"

#include "RE_InternalResources.h"
#include "RE_PrimitiveManager.h"
#include "RE_ResourceManager.h"
#include "RE_TextureImporter.h"
#include "RE_ShaderImporter.h"
#include "ModuleRenderer3D.h"

#include "RE_Mesh.h"
#include "RE_Material.h"
#include "RE_Texture.h"

#include "RE_GameObject.h"
#include "RE_CompTransform.h"

#include "ImGui\imgui.h"

RE_CompMesh::RE_CompMesh(RE_GameObject * go, const char* reference, const bool start_active)
	: RE_Component(C_MESH, go, start_active), meshMD5(reference)
{ }

RE_CompMesh::RE_CompMesh(const RE_CompMesh & cmpMesh, RE_GameObject * go)
	: RE_Component(C_MESH, go, cmpMesh.active), meshMD5(cmpMesh.meshMD5), materialMD5(cmpMesh.materialMD5)
{ }

RE_CompMesh::~RE_CompMesh()
{
}

void RE_CompMesh::Draw()
{
	const char* materialToDraw = (materialMD5) ? materialMD5 : App->internalResources->GetDefaulMaterial();
	RE_Material* material = (RE_Material *)App->resources->At(materialToDraw);
	material->UploadToShader(go->GetTransform()->GetShaderModel(), show_checkers);
	((RE_Mesh*)App->resources->At(meshMD5))->DrawMesh(material->GetShaderID());
}

void RE_CompMesh::DrawProperties()
{
	if (ImGui::CollapsingHeader("Mesh"))
	{
		if (meshMD5)
		{
			if (ImGui::Button(std::string("Resource " + std::string(App->resources->At(meshMD5)->GetName())).c_str()))
				App->resources->PushSelected(meshMD5, true);
		}
		else ImGui::TextWrapped("Empty Mesh Component");
	}

	if (ImGui::CollapsingHeader("Material"))
	{
		if (materialMD5)
		{
			ImGui::Checkbox("Use checkers texture", &show_checkers);

			if (ImGui::Button(std::string("Resource " + std::string(App->resources->At(materialMD5)->GetName())).c_str()))
				App->resources->PushSelected(materialMD5, true);
		}
		else
			ImGui::Text("Mesh don't contain Material.");
	}
}

void RE_CompMesh::SetMaterial(const char * md5)
{
	materialMD5 = md5;
}

const char * RE_CompMesh::GetMaterial() const
{
	return materialMD5;
}

std::vector<const char*> RE_CompMesh::GetAllResources()
{
	std::vector<const char*> ret;

	if (meshMD5) ret.push_back(meshMD5);
	if (materialMD5) ret.push_back(materialMD5);

	return ret;
}

unsigned int RE_CompMesh::GetBinarySize() const
{
	return sizeof(int) * 2;
}

void RE_CompMesh::SerializeJson(JSONNode* node, std::map<const char*, int>* resources)
{
	node->PushInt("meshResource", (meshMD5) ? resources->at(meshMD5) : -1);
	node->PushInt("materialResource", (materialMD5) ? resources->at(materialMD5) : -1);
}

void RE_CompMesh::SerializeBinary(char*& cursor, std::map<const char*, int>* resources)
{
	size_t size = sizeof(int);
	int md5 = (meshMD5) ? resources->at(meshMD5) : -1;
	memcpy(cursor, &md5, size);
	cursor += size;

	md5 = (materialMD5) ? resources->at(materialMD5) : -1;
	memcpy(cursor, &md5, size);
	cursor += size;
}

math::AABB RE_CompMesh::GetAABB() const
{
	return ((RE_Mesh*)App->resources->At(meshMD5))->GetAABB();
}

bool RE_CompMesh::CheckFaceCollision(const math::Ray &ray, float & distance) const
{
	math::Ray local_ray = ray;
	local_ray.Transform(go->GetTransform()->GetMatrixModel().Transposed().Inverted());
	return ((RE_Mesh*)App->resources->At(meshMD5))->CheckFaceCollision(local_ray, distance);
}

void RE_CompMesh::UseResources()
{
	if (meshMD5) App->resources->Use(meshMD5);
	if (materialMD5) {
		App->resources->Use(materialMD5);
		((RE_Material*)App->resources->At(materialMD5))->UseTextureResources();
	}
}

void RE_CompMesh::UnUseResources()
{
	if (meshMD5) App->resources->UnUse(meshMD5);
	if (materialMD5) {
		App->resources->UnUse(materialMD5);
		((RE_Material*)App->resources->At(materialMD5))->UnUseTextureResources();
	}
}

