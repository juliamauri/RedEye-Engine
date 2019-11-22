#include "RE_CompMesh.h"

#include "Application.h"
#include "ModuleScene.h"

#include "RE_Math.h"
#include "FileSystem.h"

#include "RE_InternalResources.h"
#include "RE_PrimitiveManager.h"
#include "ResourceManager.h"
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
{
	if (!(shaderForDraw = App->renderer3d->GetShaderScene()))
		shaderForDraw = App->internalResources->GetDefaultShader();
	checkerTexture = App->internalResources->GetTextureChecker();
}

RE_CompMesh::RE_CompMesh(const RE_CompMesh & cmpMesh, RE_GameObject * go)
	: RE_Component(C_MESH, go, cmpMesh.active), meshMD5(cmpMesh.meshMD5)
{
	if (!(shaderForDraw = App->renderer3d->GetShaderScene()))
		shaderForDraw = App->internalResources->GetDefaultShader();
	checkerTexture = App->internalResources->GetTextureChecker();
}

RE_CompMesh::~RE_CompMesh()
{
	//((ResourceManager*)App->meshes)->UnReference(reference);
}

void RE_CompMesh::Draw()
{
	((RE_Mesh*)App->resources->At(meshMD5))->DrawMesh(go->GetTransform()->GetShaderModel(), shaderForDraw, materialMD5, checkerTexture, show_checkers);
}

void RE_CompMesh::DrawProperties()
{
	if (ImGui::CollapsingHeader("Mesh"))
	{
		if (meshMD5)
		{
			RE_Mesh* mesh = (RE_Mesh*)App->resources->At(meshMD5);
			mesh->DrawPropieties();

			if (ImGui::Button(show_f_normals ? "Hide Face Normals" : "Show Face Normals"))
				show_f_normals = !show_f_normals;
			if (ImGui::Button(show_v_normals ? "Hide Vertex Normals" : "Show Vertex Normals"))
				show_v_normals = !show_v_normals;
			if (show_f_normals && !mesh->lFaceNormals)	mesh->loadFaceNormals();
			if (show_v_normals && !mesh->lVertexNormals)	mesh->loadVertexNormals();
			if (!show_f_normals && mesh->lFaceNormals) mesh->clearFaceNormals();
			if (!show_v_normals && mesh->lVertexNormals) mesh->clearVertexNormals();
		}
		else ImGui::TextWrapped("Empty Mesh Component");
	}

	if (ImGui::CollapsingHeader("Material"))
	{
		if (materialMD5)
		{
			ImGui::Checkbox("Use checkers texture", &show_checkers);

			ResourceContainer* meshMaterial = App->resources->At(materialMD5);
			if (meshMaterial)  meshMaterial->DrawPropieties();
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

void RE_CompMesh::Serialize(JSONNode * node, rapidjson::Value * comp_array)
{
	rapidjson::Value val(rapidjson::kObjectType);

	val.AddMember(rapidjson::Value::StringRefType("type"), rapidjson::Value().SetInt((int)type), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("reference"), rapidjson::Value().SetString(meshMD5, node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());
	val.AddMember(rapidjson::Value::StringRefType("file"), rapidjson::Value().SetString(((ResourceContainer*)App->resources->At(meshMD5))->GetLibraryPath(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());

	rapidjson::Value texture_array(rapidjson::kArrayType);
	RE_Material* meshMaterial = (RE_Material*)App->resources->At(materialMD5);
	if (meshMaterial)
	{
		rapidjson::Value texture_val(rapidjson::kObjectType);
		texture_val.AddMember(rapidjson::Value::StringRefType("path"), rapidjson::Value().SetString(((ResourceContainer*)meshMaterial)->GetLibraryPath(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());
		texture_val.AddMember(rapidjson::Value::StringRefType("md5"), rapidjson::Value().SetString(((ResourceContainer*)meshMaterial)->GetMD5(), node->GetDocument()->GetAllocator()), node->GetDocument()->GetAllocator());
		texture_array.PushBack(texture_val, node->GetDocument()->GetAllocator());
	}
	val.AddMember(rapidjson::Value::StringRefType("material"), texture_array, node->GetDocument()->GetAllocator());

	comp_array->PushBack(val, node->GetDocument()->GetAllocator());
}

math::AABB RE_CompMesh::GetAABB() const
{
	return ((RE_Mesh*)App->resources->At(meshMD5))->GetAABB();
}

