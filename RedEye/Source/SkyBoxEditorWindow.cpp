#include "SkyBoxEditorWindow.h"

#include "RE_Memory.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"
#include "RE_SkyBox.h"

#include <ImGuiImpl/imgui_stdlib.h>
#include <ImGui/imgui_internal.h>
#include <Glew/glew.h>

SkyBoxEditorWindow::SkyBoxEditorWindow() :
	EditorWindow("Skybox Editor", false),
	editingSkybox(new RE_SkyBox),
	sbName("New Skybox")
{}

SkyBoxEditorWindow::~SkyBoxEditorWindow()
{
	DEL(editingSkybox);
	if (previewImage != 0) glDeleteTextures(1, &previewImage);
}

void SkyBoxEditorWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, &active, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		ImGui::Text("Skybox name:");
		ImGui::SameLine();
		ImGui::InputText("##sbname", &sbName);

		assetPath = "Assets/Skyboxes/";
		assetPath += sbName;
		assetPath += ".sk";
		ImGui::Text("Save path: %s", assetPath.c_str());

		bool isTexturesFilled = editingSkybox->isFacesFilled();
		bool exits = RE_FS->Exists(assetPath.c_str());
		if (exits) ImGui::Text("This skybox exits, change the name.");

		if (isTexturesFilled) ImGui::Text("Needed set all textures before save.");

		if ((exits || !isTexturesFilled) && !secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		ImGui::SameLine();
		if (ImGui::Button("Save") && !sbName.empty() && !exits)
		{
			editingSkybox->SetName(sbName.c_str());
			editingSkybox->SetAssetPath(assetPath.c_str());
			editingSkybox->SetType(Resource_Type::R_SKYBOX);
			editingSkybox->AssetSave();
			editingSkybox->SaveMeta();

			RE_RENDER->PushThumnailRend(RE_RES->Reference((ResourceContainer*)editingSkybox));

			editingSkybox = new RE_SkyBox();
			sbName = "New Skybox";
		}

		if ((exits || !isTexturesFilled) && !secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		ImGui::Separator();
		editingSkybox->DrawEditSkyBox();
	}

	ImGui::End();
}
