#include "RE_CompMesh.h"

#include "Application.h"
#include "MeshManager.h"
#include "FileSystem.h"
#include "ShaderManager.h"
#include "ModuleScene.h"
#include "ResourceManager.h"
#include "RE_Mesh.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "Texture2DManager.h"
#include"RE_Math.h"
#include "ImGui\imgui.h"


RE_CompMesh::RE_CompMesh(RE_GameObject * go, const char* reference, const bool start_active) : RE_Component(C_MESH, go, start_active)
{
	this->reference = reference;
}

RE_CompMesh::~RE_CompMesh()
{
	//((ResourceManager*)App->meshes)->UnReference(reference);
}

void RE_CompMesh::Draw()
{
	if (!reference.empty())
	{
		ShaderManager::use(App->scene->modelloading);
		ShaderManager::setFloat4x4(App->scene->modelloading, "model", go->GetTransform()->GetGlobalMatInvTrans().ptr());
		App->meshes->DrawMesh(reference.c_str());
	}
	if (ptr == nullptr)
		ptr = (RE_Mesh*)App->resources->At(reference.c_str());
	
	ShaderManager::use(App->scene->modelloading);
	ShaderManager::setFloat4x4(App->scene->modelloading, "model", go->GetTransform()->GetGlobalMatInvTrans().ptr());
	App->meshes->DrawMesh(ptr);

}

void RE_CompMesh::DrawProperties()
{
	if (ImGui::CollapsingHeader("Mesh"))
	{
		if (!reference.empty())
		{
			ImGui::TextWrapped("Reference: %s", reference.c_str());

			for (auto texture : ((RE_Mesh*)App->resources->At(reference.c_str()))->textures)
				ImGui::TextWrapped("Texture Reference: %s", texture.id);
		}
		else ImGui::TextWrapped("Empty Mesh Component");
	}
}

void RE_CompMesh::Serialize(JSONNode * node, rapidjson::Value * comp_array)
{
	rapidjson::Value val(rapidjson::kObjectType);

	val.AddMember(rapidjson::Value::StringRefType("type"), rapidjson::Value().SetInt((int)type), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("reference"), rapidjson::Value().SetString(reference.c_str(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("file"), rapidjson::Value().SetString(((ResourceContainer*)App->resources->At(reference.c_str()))->GetFilePath(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());

	rapidjson::Value texture_array(rapidjson::kArrayType);
	for (auto texture : ((RE_Mesh*)App->resources->At(reference.c_str()))->textures)
	{
		rapidjson::Value texture_val(rapidjson::kObjectType);
		texture_val.AddMember(rapidjson::Value::StringRefType("reference"), rapidjson::Value().SetString(texture.id.c_str(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());
		texture_val.AddMember(rapidjson::Value::StringRefType("file"), rapidjson::Value().SetString(((ResourceContainer*)App->resources->At(texture.id.c_str()))->GetFilePath(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());

		texture_array.PushBack(texture_val, node->GetDocument()->GetAllocator());
	}
	val.AddMember(rapidjson::Value::StringRefType("textures"), texture_array, node->GetDocument()->GetAllocator());

	comp_array->PushBack(val, node->GetDocument()->GetAllocator());
}

void RE_CompMesh::SetTexture(const char * reference)
{
	Texture tex;
	tex.id = reference;
	((RE_Mesh*)App->resources->At(this->reference.c_str()))->textures.push_back(tex);
}

