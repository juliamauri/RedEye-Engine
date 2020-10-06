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

RE_CompMesh::RE_CompMesh() : RE_Component(C_MESH, go)
{
}

RE_CompMesh::~RE_CompMesh()
{
}

void RE_CompMesh::SetUp(RE_GameObject* parent, const char* reference)
{
	go = parent;
	parent->AddComponent(this);
	meshMD5 = reference;
}

void RE_CompMesh::SetUp(const RE_CompMesh& cmpMesh, RE_GameObject* parent)
{
	go = parent;
	parent->AddComponent(this);
	meshMD5 = cmpMesh.meshMD5;
	materialMD5 = cmpMesh.materialMD5;
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
	if (ImGui::CollapsingHeader("Component Mesh")) {
		if (meshMD5)
		{
			if (ImGui::Button(eastl::string("Resource Mesh").c_str()))
				App->resources->PushSelected(meshMD5, true);
		}
		else ImGui::TextWrapped("Empty Mesh Component");
		ImGui::Separator();
		ImGui::Checkbox("Use checkers texture", &show_checkers);
		ImGui::Separator();
		RE_Material* matRes = (materialMD5) ? (RE_Material*)App->resources->At(materialMD5) : (RE_Material*)App->resources->At(App->internalResources->GetDefaulMaterial());

		if (!materialMD5) ImGui::Text("This component mesh is using the default material.");

		if (ImGui::Button(matRes->GetName()))
			App->resources->PushSelected(matRes->GetMD5(), true);

		if (materialMD5) {
			ImGui::SameLine();
			if (ImGui::Button("Back to Default Material"))
				materialMD5 = nullptr;
		}

		if (ImGui::BeginMenu("Change material"))
		{
			eastl::vector<ResourceContainer*> materials = App->resources->GetResourcesByType(Resource_Type::R_MATERIAL);
			bool none = true;
			for (auto material : materials) {
				if (material->isInternal())
					continue;
				none = false;
				if (ImGui::MenuItem(material->GetName())) {
					if (materialMD5) {
						App->resources->UnUse(materialMD5);
						((RE_Material*)App->resources->At(materialMD5))->UnUseResources();
					}
					materialMD5 = material->GetMD5();
					if (materialMD5) {
						App->resources->Use(materialMD5);
						((RE_Material*)App->resources->At(materialMD5))->UseResources();
					}
				}
			}
			if (none) ImGui::Text("No custom materials on assets");
			ImGui::EndMenu();
		}

		if (ImGui::BeginDragDropTarget()) {

			if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload("#MaterialReference")) {
				if (materialMD5) App->resources->UnUse(materialMD5);
				materialMD5 = *static_cast<const char**>(dropped->Data);
				if (materialMD5) App->resources->Use(materialMD5);
			}
			ImGui::EndDragDropTarget();
		}
	}
}

unsigned int RE_CompMesh::GetVAOMesh() const
{
	return (meshMD5) ? ((RE_Mesh*)App->resources->At(meshMD5))->GetVAO() : 0;
}

unsigned int RE_CompMesh::GetTriangleMesh() const
{
	return (meshMD5) ? ((RE_Mesh*)App->resources->At(meshMD5))->GetTriangleCount() : 0;
}

void RE_CompMesh::SetMaterial(const char * md5)
{
	materialMD5 = md5;
}

const char * RE_CompMesh::GetMaterial() const
{
	return materialMD5;
}

eastl::vector<const char*> RE_CompMesh::GetAllResources()
{
	eastl::vector<const char*> ret;

	if (meshMD5) ret.push_back(meshMD5);
	if (materialMD5) ret.push_back(materialMD5);

	return ret;
}

unsigned int RE_CompMesh::GetBinarySize() const
{
	return sizeof(int) * 2;
}

void RE_CompMesh::SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources)
{
	node->PushInt("meshResource", (meshMD5) ? resources->at(meshMD5) : -1);
	node->PushInt("materialResource", (materialMD5) ? resources->at(materialMD5) : -1);
}

void RE_CompMesh::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources)
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
		((RE_Material*)App->resources->At(materialMD5))->UseResources();
	}
}

void RE_CompMesh::UnUseResources()
{
	if (meshMD5) App->resources->UnUse(meshMD5);
	if (materialMD5) {
		App->resources->UnUse(materialMD5);
		((RE_Material*)App->resources->At(materialMD5))->UnUseResources();
	}
}

bool RE_CompMesh::isBlend() const
{
	const char* materialToDraw = (materialMD5) ? materialMD5 : App->internalResources->GetDefaulMaterial();
	RE_Material* material = (RE_Material*)App->resources->At(materialToDraw);
	return material->blendMode;
}

