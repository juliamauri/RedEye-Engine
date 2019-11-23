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

unsigned int RE_CompMesh::GetBinarySize() const
{
	uint count = 0;
	if (meshMD5)count++;
	if (materialMD5)count++;
	return sizeof(int) * count;
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

