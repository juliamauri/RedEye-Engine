#include "EditorWindow.h"
#include <EASTL/string.h>

#include "WaterPlaneWindow.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"
#include "RE_FileBuffer.h"
#include "RE_DefaultShaders.h"
#include "RE_Material.h"
#include "RE_Shader.h"

#include <ImGui/imgui_internal.h>
#include <ImGuiImpl/imgui_stdlib.h>

void WaterPlaneWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		ImGui::TextWrapped("After generating resource, create a primitive plane, set slices and stacks and finally transform as mesh, then you can add the new material.\n\
			Remember to set the values and save on the generated material. Important to set direction different from 0 - 0.");

		ImGui::Checkbox("Deferred", &deferred);

		eastl::string shaderPath("Assets/Shaders/");

		shaderPath += (deferred) ? "WaterDeferredShader" : "WaterShader";
		shaderPath += ".meta";

		const char* waterShader = RE_RES->FindMD5ByMETAPath(shaderPath.c_str(), ResourceContainer::Type::SHADER);
		if (!waterShader)
		{
			ImGui::Text((deferred) ? "Water Deferred Shader doesn't exists." : "Water Shader doesn't exists.");
			ImGui::Text("Shader will generate when create.");
		}
		else ImGui::Text("Shader detected.");

		eastl::string materialPath("Assets/Materials/");
		ImGui::InputText("#WaterMaterialName", &waterResouceName);
		materialPath += waterResouceName;
		materialPath += ".pupil";

		bool exists = RE_FS->Exists(materialPath.c_str());
		if (exists) ImGui::Text("Material exists!");

		if (exists && !secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (ImGui::Button("Create water resource"))
		{
			if (!waterShader)
			{
				eastl::string shaderVertexFile("Assets/Shaders/");
				shaderVertexFile += (deferred) ? "WaterDeferred.vert" : "Water.vert";

				if (!RE_FS->Exists(shaderVertexFile.c_str()))
				{
					RE_FileBuffer vertexFile(shaderVertexFile.c_str());
					vertexFile.Save((deferred) ? WATERPASSVERTEXSHADER : WATERVERTEXSHADER,
						eastl::CharStrlen((deferred) ? WATERPASSVERTEXSHADER : WATERVERTEXSHADER));
				}

				eastl::string shaderFragmentFile("Assets/Shaders/");
				shaderFragmentFile += (deferred) ? "WaterDeferred.frag" : "Water.frag";

				if (!RE_FS->Exists(shaderFragmentFile.c_str()))
				{
					RE_FileBuffer fragmentFile(shaderFragmentFile.c_str());
					fragmentFile.Save((deferred) ? WATERPASSFRAGMENTSHADER : WATERFRAGMENTSHADER,
						eastl::CharStrlen((deferred) ? WATERPASSFRAGMENTSHADER : WATERFRAGMENTSHADER));
				}

				RE_Shader* waterShaderRes = new RE_Shader();
				waterShaderRes->SetName((deferred) ? "WaterDeferredShader" : "WaterShader");
				waterShaderRes->SetType(ResourceContainer::Type::SHADER);
				waterShaderRes->SetMetaPath(shaderPath.c_str());
				waterShaderRes->SetPaths(shaderVertexFile.c_str(), shaderFragmentFile.c_str(), nullptr);
				waterShaderRes->ShaderFilesChanged();
				waterShaderRes->SaveMeta();
				waterShader = RE_RES->Reference(static_cast<ResourceContainer*>(waterShaderRes));
			}

			RE_Material* editingMaterialRes = new RE_Material();
			editingMaterialRes->blendMode = true;
			editingMaterialRes->SetName(waterResouceName.c_str());
			editingMaterialRes->SetAssetPath(materialPath.c_str());
			editingMaterialRes->SetType(ResourceContainer::Type::MATERIAL);
			editingMaterialRes->SetShader(waterShader); //save meta after add to shader

			RE_RENDER->PushThumnailRend(RE_RES->Reference((ResourceContainer*)editingMaterialRes));

			waterResouceName = "WaterMaterial";
		}

		if (exists || secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}
