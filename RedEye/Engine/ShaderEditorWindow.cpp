#include "EditorWindow.h"
#include <EASTL/string.h>

#include "ShaderEditorWindow.h"

#include "RE_Memory.h"
#include "Application.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"
#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"
#include "RE_Shader.h"
#include "RE_DefaultShaders.h"
#include "RE_ShaderImporter.h"

#include <ImGuiImpl/imgui_stdlib.h>
#include <ImGui/imgui_internal.h>

ShaderEditorWindow::ShaderEditorWindow() :
	EditorWindow("Shader Editor", false),
	editingShader(new RE_Shader),
	shaderName("New Shader"),
	vertexPath(""),
	fragmentPath(""),
	geometryPath("")
{}

ShaderEditorWindow::~ShaderEditorWindow()
{
	DEL(editingShader);
}

void ShaderEditorWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, &active, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		ImGui::Text("Shader name:");
		ImGui::SameLine();
		ImGui::InputText("##shadername", &shaderName);

		bool nameReserved = false;
		if (shaderName == "WaterShader" || shaderName == "WaterDeferredShader") {
			nameReserved = true;
			ImGui::Text("This name is reserved!");
		}

		assetPath = "Assets/Shaders/";
		assetPath += shaderName;
		assetPath += ".meta";
		ImGui::Text("Save path: %s", assetPath.c_str());

		static bool compilePass = false;
		static bool vertexModify = false;
		static bool fragmentModify = false;
		static bool geometryModify = false;

		bool exists = RE_FS->Exists(assetPath.c_str());
		if (exists) ImGui::Text("This shader exits, change the name.");

		bool neededVertexAndFragment = (vertexPath.empty() || fragmentPath.empty());
		if (neededVertexAndFragment) ImGui::Text("The vertex or fragment file path is empty.");

		static eastl::string* waitingPath = nullptr;
		if (waitingPath) ImGui::Text("Select Shader file from assets panel");

		bool pop2 = false;
		if (!compilePass) {
			ImGui::Text("Nedded pass the compile test.");
			pop2 = true;
		}
		if ((neededVertexAndFragment || exists || pop2 || nameReserved) && !secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		ImGui::SameLine();
		if (ImGui::Button("Save")) {
			if (!shaderName.empty() && !exists) {
				editingShader->SetName(shaderName.c_str());
				editingShader->SetType(ResourceType::SHADER);
				editingShader->SetPaths(vertexPath.c_str(), fragmentPath.c_str(), (!geometryPath.empty()) ? geometryPath.c_str() : nullptr);
				editingShader->isShaderFilesChanged();
				editingShader->SaveMeta();
				RE_RES->Reference((ResourceContainer*)editingShader);

				editingShader = new RE_Shader();
				shaderName = "New Shader";
				vertexPath = "";
				fragmentPath = "";
				geometryPath = "";
				compilePass = false;
			}
		}

		if ((neededVertexAndFragment || exists || pop2 || nameReserved) && !secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		ImGui::Separator();
		ImGui::Text("The files that contains the script is an undefined file.");
		ImGui::Text("Select the script type and undefined file on panel assets.");
		ImGui::Text("When activate the selection, the undefied files from assets panel can be selected.");

		bool pop = (waitingPath);
		if (pop && !secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		ImGui::Separator();
		ImGui::Text("Vertex Shader Path:\n%s", (vertexPath.empty()) ? "No path." : vertexPath.c_str());
		if (ImGui::Button("Select vertex path"))
		{
			vertexPath.clear();
			waitingPath = &vertexPath;
			RE_EDITOR->SelectUndefinedFile(waitingPath);
			compilePass = false;
		}

		ImGui::SameLine();
		if (!vertexPath.empty() && !vertexModify)
		{
			if (ImGui::Button("Edit Vertex Shader"))
			{
				vertexModify = true;
				RE_EDITOR->OpenTextEditor(vertexPath.c_str(), &vertexPath, nullptr, &vertexModify);
			}

			ImGui::SameLine();
			if (ImGui::Button("Clear vertex")) vertexPath.clear();
		}
		else if (vertexPath.empty() && !vertexModify && !fragmentModify && !geometryModify && ImGui::Button("New vertex shader"))
		{
			vertexModify = true;
			RE_EDITOR->OpenTextEditor(nullptr, &vertexPath, DEFVERTEXSHADER, &vertexModify);
		}

		ImGui::Separator();
		ImGui::Text("Fragment Shader Path:\n%s", (fragmentPath.empty()) ? "No path." : fragmentPath.c_str());
		if (ImGui::Button("Select fragment path"))
		{
			fragmentPath.clear();
			waitingPath = &fragmentPath;
			RE_EDITOR->SelectUndefinedFile(waitingPath);
			compilePass = false;
		}

		ImGui::SameLine();
		if (!fragmentPath.empty() && !fragmentModify)
		{
			if (ImGui::Button("Edit Fragment Shader"))
			{
				fragmentModify = true;
				RE_EDITOR->OpenTextEditor(fragmentPath.c_str(), &fragmentPath, nullptr, &fragmentModify);
			}

			ImGui::SameLine();
			if (ImGui::Button("Clear fragment")) fragmentPath.clear();
		}
		else if (fragmentPath.empty() && !vertexModify && !fragmentModify && !geometryModify && ImGui::Button("New fragment shader"))
		{
			fragmentModify = true;
			RE_EDITOR->OpenTextEditor(nullptr, &fragmentPath, DEFFRAGMENTSHADER, &fragmentModify);
		}

		ImGui::Separator();
		ImGui::Text("Geometry Shader Path:\n%s", (geometryPath.empty()) ? "No path." : geometryPath.c_str());
		if (ImGui::Button("Select geometry path"))
		{
			geometryPath.clear();
			waitingPath = &geometryPath;
			RE_EDITOR->SelectUndefinedFile(waitingPath);
			compilePass = false;
		}

		ImGui::SameLine();
		if (!geometryPath.empty() && !geometryModify)
		{
			if (ImGui::Button("Edit Geometry Shader"))
			{
				geometryModify = true;
				RE_EDITOR->OpenTextEditor(geometryPath.c_str(), &geometryPath, nullptr, &geometryModify);
			}

			ImGui::SameLine();
			if (ImGui::Button("Clear geometry")) geometryPath.clear();
		}
		else if (geometryPath.empty() && !vertexModify && !fragmentModify && !geometryModify && ImGui::Button("New geometry shader"))
		{
			geometryModify = true;
			RE_EDITOR->OpenTextEditor(nullptr, &geometryPath, nullptr, &geometryModify);
		}

		ImGui::Separator();
		if (neededVertexAndFragment) ImGui::Text("The vertex or fragment file path is empty.");

		if (neededVertexAndFragment && !pop && !secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (!compilePass && ImGui::Button("Compile Test"))
		{
			RE_LOGGER::ScopeProcedureLogging();

			uint sID = 0;
			compilePass = RE_ShaderImporter::LoadFromAssets(&sID, vertexPath.c_str(), fragmentPath.c_str(), (!geometryPath.empty()) ? geometryPath.c_str() : nullptr, true);
			if (!compilePass) RE_LOG_ERROR("Shader Compilation Error:\n%s", RE_ShaderImporter::GetShaderError());

			RE_LOGGER::EndScope();
		}

		if (secondary || pop || neededVertexAndFragment)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		if (waitingPath) waitingPath = nullptr;
	}

	ImGui::End();
}
