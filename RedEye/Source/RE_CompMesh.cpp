#include "RE_CompMesh.h"

#include "Application.h"
#include "FileSystem.h"
#include "ShaderManager.h"
#include "ModuleScene.h"
#include "ResourceManager.h"
#include "RE_Mesh.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_TextureImporter.h"
#include "RE_Material.h"
#include "RE_Math.h"
#include "RE_PrimitiveManager.h"
#include "ImGui\imgui.h"


RE_CompMesh::RE_CompMesh(RE_GameObject * go, const char* reference, const bool start_active)
	: RE_Component(C_MESH, go, start_active), reference(reference)
{
	ptr = (RE_Mesh*)App->resources->At(this->reference.c_str());
}

RE_CompMesh::RE_CompMesh(const RE_CompMesh & cmpMesh, RE_GameObject * go)
	: RE_Component(C_MESH, go, cmpMesh.active), reference(cmpMesh.reference)
{
	ptr = (RE_Mesh*)App->resources->At(this->reference.c_str());
}

RE_CompMesh::~RE_CompMesh()
{
	//((ResourceManager*)App->meshes)->UnReference(reference);
}

void RE_CompMesh::Draw()
{
	ptr->Draw(go->GetTransform()->GetShaderModel(), show_checkers);
}

void RE_CompMesh::DrawProperties()
{
	if (ImGui::CollapsingHeader("Mesh"))
	{
		if (!reference.empty())
		{
			ImGui::Text("Name: %s", ptr->GetName());
			ImGui::TextWrapped("Reference: %s", reference.c_str());
			ImGui::TextWrapped("Directory: %s", ptr->GetFilePath());
			ImGui::Text("Vertex count: %u", ptr->vertices.size());
			ImGui::Text("Triangle Face count: %u", ptr->triangle_count);
			ImGui::Text("VAO: %u", ptr->VAO);

			if (ImGui::TreeNodeEx("Bounding Box"))
			{
				math::AABB box = ptr->GetAABB();
				ImGui::TextWrapped("Min: { %.2f, %.2f, %.2f}", box.minPoint.x, box.minPoint.y, box.minPoint.z);
				ImGui::TextWrapped("Max: { %.2f, %.2f, %.2f}", box.maxPoint.x, box.maxPoint.y, box.maxPoint.z);
				ImGui::TreePop();
			}

			if (ImGui::Button(show_f_normals ? "Hide Face Normals" : "Show Face Normals"))
				show_f_normals = !show_f_normals;
			if (ImGui::Button(show_v_normals ? "Hide Vertex Normals" : "Show Vertex Normals"))
				show_v_normals = !show_v_normals;
			if (show_f_normals && !ptr->lFaceNormals)	ptr->loadFaceNormals();
			if (show_v_normals && !ptr->lVertexNormals)	ptr->loadVertexNormals();
			if (!show_f_normals && ptr->lFaceNormals) ptr->clearFaceNormals();
			if (!show_v_normals && ptr->lVertexNormals) ptr->clearVertexNormals();
		}
		else ImGui::TextWrapped("Empty Mesh Component");
	}

	if (ImGui::CollapsingHeader("Material"))
	{
		if (!reference.empty() && ptr->materialMD5)
		{
			ImGui::Checkbox("Use checkers texture", &show_checkers);

			RE_Material* meshMaterial = (RE_Material*)App->resources->At(ptr->materialMD5);
			if (meshMaterial && !meshMaterial->tDiffuse.empty())
			{
				for (unsigned int i = 0; i < meshMaterial->tDiffuse.size(); i++)
				{
					int width = 0;
					int height = 0;
					Texture2D* texture = (Texture2D*)App->resources->At(meshMaterial->tDiffuse[i]);
					texture->GetWithHeight(&width, &height);

					ImGui::Text("Type: Diffuse");
					ImGui::Text("Size: %ux%u", width, height);
					ImGui::TextWrapped("Path: %s", texture->GetFilePath());
					ImGui::Text("MD5: %s", texture->GetMD5());
					texture->DrawTextureImGui();
				}
			}
		}
		else
			ImGui::Text("Mesh don't contain Material.");
	}
}

void RE_CompMesh::SetMaterial(const char * md5)
{
	ptr->materialMD5 = md5;
}

const char * RE_CompMesh::GetMaterial() const
{
	return ptr->materialMD5;
}

void RE_CompMesh::Serialize(JSONNode * node, rapidjson::Value * comp_array)
{
	rapidjson::Value val(rapidjson::kObjectType);

	val.AddMember(rapidjson::Value::StringRefType("type"), rapidjson::Value().SetInt((int)type), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("reference"), rapidjson::Value().SetString(reference.c_str(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("file"), rapidjson::Value().SetString(((ResourceContainer*)App->resources->At(reference.c_str()))->GetFilePath(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());

	rapidjson::Value texture_array(rapidjson::kArrayType);
	RE_Material* meshMaterial = (RE_Material*)App->resources->At(ptr->materialMD5);
	if (meshMaterial)
	{
		rapidjson::Value texture_val(rapidjson::kObjectType);
		texture_val.AddMember(rapidjson::Value::StringRefType("path"), rapidjson::Value().SetString(((ResourceContainer*)meshMaterial)->GetFilePath(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());
		texture_val.AddMember(rapidjson::Value::StringRefType("md5"), rapidjson::Value().SetString(((ResourceContainer*)meshMaterial)->GetMD5(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());
		texture_array.PushBack(texture_val, node->GetDocument()->GetAllocator());
	}
	val.AddMember(rapidjson::Value::StringRefType("material"), texture_array, node->GetDocument()->GetAllocator());

	comp_array->PushBack(val, node->GetDocument()->GetAllocator());
}

math::AABB RE_CompMesh::GetAABB() const
{
	return ptr->GetAABB();
}

