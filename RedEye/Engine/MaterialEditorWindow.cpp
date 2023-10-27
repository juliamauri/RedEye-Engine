#include "MaterialEditorWindow.h"

#include "RE_Memory.h"
#include "Application.h"
#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"
#include "RE_ThumbnailManager.h"
#include "RE_Material.h"

#include <ImGuiImpl/imgui_stdlib.h>
#include <ImGui/imgui_internal.h>

MaterialEditorWindow::MaterialEditorWindow() :
	EditorWindow("Material Editor", false),
	editing_material(new RE_Material),
	matName("New Material")
{}

MaterialEditorWindow::~MaterialEditorWindow()
{
	DEL(editing_material)
}

void MaterialEditorWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, &active, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		ImGui::Text("Material name:");
		ImGui::SameLine();
		ImGui::InputText("##matname", &matName);

		assetPath = "Assets/Materials/";
		assetPath += matName;
		assetPath += ".pupil";
		ImGui::Text("Save path: %s", assetPath.c_str());

		bool exits = RE_FS->Exists(assetPath.c_str());
		if (exits) ImGui::Text("This material exits, change the name.");

		if (exits && !secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		ImGui::SameLine();
		if (ImGui::Button("Save") && !matName.empty() && !exits)
		{
			editing_material->SetName(matName.c_str());
			editing_material->SetAssetPath(assetPath.c_str());
			editing_material->SetType(ResourceContainer::Type::MATERIAL);
			editing_material->Save();

			RE_ThumbnailManager::AddThumbnail(RE_RES->Reference(editing_material));

			editing_material = new RE_Material();
			matName = "New Material";
		}

		if (exits && !secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		ImGui::Separator();
		editing_material->DrawMaterialEdit();

		if (secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}

	ImGui::End();
}