#include "RE_CompMesh.h"

#include "Application.h"
#include "MeshManager.h"
#include "ShaderManager.h"
#include "ModuleScene.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
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

}

void RE_CompMesh::DrawProperties()
{
	if (ImGui::CollapsingHeader("Mesh"))
	{
		if (!reference.empty()) ImGui::TextWrapped("Reference: %s",reference.c_str());
		else ImGui::TextWrapped("Empty Mesh Component");
	}
}

