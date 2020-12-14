#include "RE_CompMesh.h"

#include "RE_Math.h"
#include "RE_ConsoleLog.h"
#include "RE_Json.h"
#include "Application.h"
#include "RE_ResourceManager.h"
#include "RE_InternalResources.h"
#include "RE_ECS_Pool.h"
#include "RE_Mesh.h"
#include "RE_Material.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"

#include "ImGui\imgui.h"

void RE_CompMesh::CopySetUp(GameObjectsPool* pool, RE_Component* copy, const UID parent)
{
	pool_gos = pool;
	if (go = parent) pool_gos->AtPtr(go)->ReportComponent(id, C_MESH);

	RE_CompMesh* cmpMesh = dynamic_cast<RE_CompMesh*>(copy);
	meshMD5 = cmpMesh->meshMD5;
	materialMD5 = cmpMesh->materialMD5;
}

void RE_CompMesh::Draw() const
{
	const char* materialToDraw = (materialMD5) ? materialMD5 : RE_RES->internalResources->GetDefaulMaterial();
	RE_Material* material = dynamic_cast<RE_Material*>(RE_RES->At(materialToDraw));
	if (material != nullptr)
	{
		material->UploadToShader(GetGOCPtr()->GetTransformPtr()->GetGlobalMatrixPtr(), show_checkers);
		RE_Mesh* mesh = dynamic_cast<RE_Mesh*>(RE_RES->At(meshMD5));
		if (mesh != nullptr) mesh->DrawMesh(material->GetShaderID());
		else RE_LOG_WARNING("Component Mesh tried drawing invalid mesh");
	}
	else RE_LOG_WARNING("Component Mesh tried drawing invalid material");
}

void RE_CompMesh::DrawProperties()
{
	if (ImGui::CollapsingHeader("Component Mesh"))
	{
		if (meshMD5)
		{
			if (ImGui::Button(eastl::string("Resource Mesh").c_str()))
				RE_RES->PushSelected(meshMD5, true);
		}
		else ImGui::TextWrapped("Empty Mesh Component");

		ImGui::Separator();
		ImGui::Checkbox("Use checkers texture", &show_checkers);
		ImGui::Separator();
		RE_Material* matRes = (materialMD5) ?
			dynamic_cast<RE_Material*>(RE_RES->At(materialMD5)) :
			dynamic_cast<RE_Material*>(RE_RES->At(RE_RES->internalResources->GetDefaulMaterial()));

		if (!materialMD5) ImGui::Text("This component mesh is using the default material.");
		if (ImGui::Button(matRes->GetName())) RE_RES->PushSelected(matRes->GetMD5(), true);
		if (materialMD5)
		{
			ImGui::SameLine();
			if (ImGui::Button("Back to Default Material")) materialMD5 = nullptr;
		}

		if (ImGui::BeginMenu("Change material"))
		{
			eastl::vector<ResourceContainer*> materials = RE_RES->GetResourcesByType(Resource_Type::R_MATERIAL);
			bool none = true;
			for (auto material : materials)
			{
				if (material->isInternal()) continue;

				none = false;
				if (ImGui::MenuItem(material->GetName()))
				{
					if (materialMD5)
					{
						RE_RES->UnUse(materialMD5);
						(dynamic_cast<RE_Material*>(RE_RES->At(materialMD5)))->UnUseResources();
					}
					materialMD5 = material->GetMD5();
					if (materialMD5)
					{
						RE_RES->Use(materialMD5);
						(dynamic_cast<RE_Material*>(RE_RES->At(materialMD5)))->UseResources();
					}
				}
			}
			if (none) ImGui::Text("No custom materials on assets");
			ImGui::EndMenu();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload("#MaterialReference"))
			{
				if (materialMD5) RE_RES->UnUse(materialMD5);
				materialMD5 = *static_cast<const char**>(dropped->Data);
				if (materialMD5) RE_RES->Use(materialMD5);
			}
			ImGui::EndDragDropTarget();
		}
	}
}

void RE_CompMesh::SetMesh(const char* mesh)
{
	meshMD5 = mesh;
	if (go) GetGOPtr()->ResetBoundingBoxes();
}

const char* RE_CompMesh::GetMesh() const
{
	return meshMD5;
}

unsigned int RE_CompMesh::GetVAOMesh() const
{
	return (meshMD5) ? (dynamic_cast<RE_Mesh*>(RE_RES->At(meshMD5)))->GetVAO() : 0u;
}

unsigned int RE_CompMesh::GetTriangleMesh() const
{
	return (meshMD5) ? (dynamic_cast<RE_Mesh*>(RE_RES->At(meshMD5)))->GetTriangleCount() : 0u;
}

void RE_CompMesh::SetMaterial(const char * md5) { materialMD5 = md5; }
const char * RE_CompMesh::GetMaterial() const { return materialMD5; }

eastl::vector<const char*> RE_CompMesh::GetAllResources()
{
	eastl::vector<const char*> ret;
	if (meshMD5) ret.push_back(meshMD5);
	if (materialMD5) ret.push_back(materialMD5);
	return ret;
}

void RE_CompMesh::SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources) const
{
	node->PushInt("meshResource", (meshMD5) ? resources->at(meshMD5) : -1);
	node->PushInt("materialResource", (materialMD5) ? resources->at(materialMD5) : -1);
}

void RE_CompMesh::DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources)
{
	int id = node->PullInt("meshResource", -1);
	meshMD5 = (id != -1) ? resources->at(id) : nullptr;
	id = node->PullInt("materialResource", -1);
	materialMD5 = (id != -1) ? resources->at(id) : nullptr;
}

unsigned int RE_CompMesh::GetBinarySize() const
{
	return sizeof(int) * 2;
}

void RE_CompMesh::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const
{
	size_t size = sizeof(int);
	int md5 = (meshMD5) ? resources->at(meshMD5) : -1;
	memcpy(cursor, &md5, size);
	cursor += size;

	md5 = (materialMD5) ? resources->at(materialMD5) : -1;
	memcpy(cursor, &md5, size);
	cursor += size;
}

void RE_CompMesh::DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources)
{
	size_t size = sizeof(int);
	int md5 = -1;

	memcpy(&md5, cursor, size);
	cursor += size;
	meshMD5 = (md5 != -1 ) ? resources->at(md5) : nullptr;

	md5 = -1;
	memcpy(&md5, cursor,  size);
	cursor += size;
	materialMD5 = (md5 != -1) ? resources->at(md5) : nullptr;
}

math::AABB RE_CompMesh::GetAABB() const
{
	math::AABB ret;
	RE_Mesh* mesh;
	if (meshMD5 && (mesh = dynamic_cast<RE_Mesh*>(RE_RES->At(meshMD5)))) ret = mesh->GetAABB();
	else ret.SetFromCenterAndSize(math::vec::zero, math::vec::zero);
	return ret;
}

bool RE_CompMesh::CheckFaceCollision(const math::Ray & local_ray, float & distance) const
{
	return meshMD5 && (dynamic_cast<RE_Mesh*>(RE_RES->At(meshMD5)))->CheckFaceCollision(local_ray, distance);
}

void RE_CompMesh::UseResources()
{
	if (meshMD5) RE_RES->Use(meshMD5);
	if (materialMD5)
	{
		RE_RES->Use(materialMD5);
		(dynamic_cast<RE_Material*>(RE_RES->At(materialMD5)))->UseResources();
	}
}

void RE_CompMesh::UnUseResources()
{
	if (meshMD5) RE_RES->UnUse(meshMD5);
	if (materialMD5)
	{
		RE_RES->UnUse(materialMD5);
		(dynamic_cast<RE_Material*>(RE_RES->At(materialMD5)))->UnUseResources();
	}
}

bool RE_CompMesh::isBlend() const
{
	const char* materialToDraw = (materialMD5) ? materialMD5 : RE_RES->internalResources->GetDefaulMaterial();
	RE_Material* material = dynamic_cast<RE_Material*>(RE_RES->At(materialToDraw));
	return material->blendMode;
}

