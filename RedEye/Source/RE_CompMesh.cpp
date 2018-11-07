#include "RE_CompMesh.h"

#include "Application.h"
#include "MeshManager.h"
#include "ImGui\imgui.h"

RE_CompMesh::RE_CompMesh(RE_GameObject * go, const char * path, const bool file_dropped, const bool start_active) : RE_Component(C_MESH, go, start_active)
{
	LoadMesh(path, file_dropped);
}

RE_CompMesh::RE_CompMesh(RE_GameObject * go, unsigned int reference, const bool start_active) : RE_Component(C_MESH, go, start_active)
{
	this->reference = reference;
}

RE_CompMesh::~RE_CompMesh()
{
	((ResourceManager*)App->meshes)->UnReference(reference);
}

unsigned int RE_CompMesh::LoadMesh(const char * path, bool dropped)
{
	((ResourceManager*)App->meshes)->UnReference(reference);

	if (path != nullptr)
		reference = App->meshes->LoadMesh(path, dropped);

	return reference;
}

void RE_CompMesh::Draw()
{
	if(reference) App->meshes->DrawMesh(reference);
}

void RE_CompMesh::DrawProperties()
{
	if (ImGui::CollapsingHeader("Mesh"))
	{
		if (reference) App->meshes->DrawMesh(reference);
		else ImGui::TextWrapped("Empty Mesh Component");
	}
}

