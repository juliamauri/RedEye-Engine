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

	if (ptr == nullptr)
		ptr = (RE_Mesh*)App->resources->At(this->reference.c_str());
}

RE_CompMesh::~RE_CompMesh()
{
	//((ResourceManager*)App->meshes)->UnReference(reference);
}

void RE_CompMesh::Draw()
{
	ShaderManager::use(App->scene->modelloading);
	ShaderManager::setFloat4x4(App->scene->modelloading, "model", go->GetTransform()->GetShaderModel());
	App->meshes->DrawMesh(ptr);

}

void RE_CompMesh::DrawProperties()
{
	if (ImGui::CollapsingHeader("Mesh"))
	{
		if (!reference.empty())
		{

			//ImGui::Text("Name: %s", ptr->GetName());
			ImGui::TextWrapped("Reference: %s", reference.c_str());

			ImGui::TextWrapped("Directory: %s", ptr->GetFilePath());
			//ImGui::Text("Triangle Count: %u", total_triangle_count);

			//if (ImGui::Button(show_f_normals ? "Hide Face Normals" : "Show Face Normals")) show_f_normals = !show_f_normals;

			//if (ImGui::Button(show_v_normals ? "Hide Vertex Normals" : "Show Vertex Normals")) show_v_normals = !show_v_normals;

			int width = 0;
			int height = 0;
			//if (show_f_normals && !ptr->lFaceNormals)	ptr->loadFaceNormals();
			//if (show_v_normals && !ptr->lVertexNormals)	ptr->loadVertexNormals();

			//if (!show_f_normals && ptr->lFaceNormals) ptr->clearFaceNormals();
			//if (!show_v_normals && ptr->lVertexNormals) ptr->clearVertexNormals();


			ImGui::Text("Vertex count: %u", ptr->vertices.size());
			ImGui::Text("Triangle Face count: %u", ptr->triangle_count);
			ImGui::Text("VAO: %u", ptr->VAO);

			std::vector<Texture>::iterator it = ptr->textures.begin();
			for (unsigned int i = 1; it != ptr->textures.end(); it++, i++)
			{
				Texture2D* texture = it->ptr;
				texture->GetWithHeight(&width, &height);

				if (ImGui::TreeNode("Texture"))
				{
					ImGui::Text("\t- MD5: %s", texture->GetMD5());
					ImGui::Text("\t- Size: %ux%u", width, height);
					ImGui::TextWrapped("\t- Path: %s", texture->GetFilePath());
					ImGui::Text("\t- Type: %s", it->type.c_str());

					texture->DrawTextureImGui();

					ImGui::TreePop();
				}
			}
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
	for (auto texture : ptr->textures)
	{
		rapidjson::Value texture_val(rapidjson::kObjectType);
		texture_val.AddMember(rapidjson::Value::StringRefType("reference"), rapidjson::Value().SetString(texture.id.c_str(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());
		texture_val.AddMember(rapidjson::Value::StringRefType("file"), rapidjson::Value().SetString(texture.path.c_str(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());
		texture_val.AddMember(rapidjson::Value::StringRefType("type"), rapidjson::Value().SetString(texture.type.c_str(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());

		texture_array.PushBack(texture_val, node->GetDocument()->GetAllocator());
	}
	val.AddMember(rapidjson::Value::StringRefType("textures"), texture_array, node->GetDocument()->GetAllocator());

	comp_array->PushBack(val, node->GetDocument()->GetAllocator());
}

void RE_CompMesh::SetTexture(const char * reference, const char* file_path, const char* type)
{
	bool have_texture = false;
	if (ptr->textures.size() > 0)
	{
		for (auto texture : ptr->textures)
			if (texture.id.compare(reference) == 0)
			{
				have_texture = true;
				break;
			}
	}

	if (!have_texture)
	{
		Texture tex;
		const char* is_reference = App->resources->IsReference(reference);
		if (is_reference)
		{
			tex.path = file_path;
			tex.type = type;
			tex.id = is_reference;
			tex.ptr = (Texture2D*)App->resources->At(is_reference);
			ptr->textures.push_back(tex);
		}
	}
}

