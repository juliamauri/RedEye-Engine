#include "EditorWindows.h"

#include "Application.h"
#include "RE_FileSystem.h"
#include "ModuleEditor.h"
#include "ModuleScene.h"
#include "RE_ThumbnailManager.h"
#include "RE_ShaderImporter.h"
#include "RE_ResourceManager.h"

#include "RE_InternalResources.h"
#include "RE_Material.h"
#include "RE_SkyBox.h"
#include "RE_Shader.h"
#include "RE_Prefab.h"

#include "RE_GameObject.h"

#include "OutputLog.h"
#include "RE_HandleErrors.h"
#include "RE_DefaultShaders.h"
#include "Globals.h"

#include "ImGui/misc/cpp/imgui_stdlib.h"
#include "ImGui/imgui_internal.h"
#include "ImGuiColorTextEdit/TextEditor.h"

#include "Glew/include/glew.h"


//Needed to redo, deprecated
PrefabsPanel::PrefabsPanel(const char* name, bool start_active) : EditorWindow(name, start_active) {}

PrefabsPanel::~PrefabsPanel()
{
}

void PrefabsPanel::Draw(bool secondary)
{
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		ImGui::SameLine();

		if (ImGui::Button("Create Prefab"))
		{
			if (App->editor->GetSelected() != nullptr)
			{
				RE_Prefab* newPrefab = new RE_Prefab();
				RE_GameObject* selected = App->editor->GetSelected();
				newPrefab->SetName(selected->GetName());
				newPrefab->Save(selected);
				newPrefab->SaveMeta();
				App->resources->Reference(newPrefab);
				prefabs = App->resources->GetResourcesByType(Resource_Type::R_PREFAB);
			}
		}

		if (ImGui::Button("Clone selected to Scene"))
		{
			if (selected != nullptr)
				App->scene->AddGoToRoot(((RE_Prefab*)selected)->GetRoot());
		}

		ImGui::NewLine();

		if (!prefabs.empty())
		{
			ImGui::Text("Current Prefabs");

			for (ResourceContainer* currentPrefab : prefabs)
			{
				if (ImGui::Button((selected == currentPrefab) ? std::string(std::string("Selected -> ") + std::string(currentPrefab->GetName())).c_str() : currentPrefab->GetName()))
					selected = currentPrefab;
			}

		}
		else
			ImGui::Text("Empty");

		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}

MaterialEditorWindow::MaterialEditorWindow(const char* name, bool start_active) : EditorWindow(name, start_active)
{
	editingMaerial = new RE_Material();
	matName = "New Material";
}

MaterialEditorWindow::~MaterialEditorWindow()
{
	DEL(editingMaerial);
}

void MaterialEditorWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
	{
		if (secondary) {
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

		bool exits = App->fs->Exists(assetPath.c_str());
		if (exits) ImGui::Text("This material exits, change the name.");

		if (exits && !secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		ImGui::SameLine();
		if (ImGui::Button("Save")) {
			if (!matName.empty() && !exits) {
				editingMaerial->SetName(matName.c_str());
				editingMaerial->SetAssetPath(assetPath.c_str());
				editingMaerial->SetType(Resource_Type::R_MATERIAL);
				editingMaerial->Save();

				App->thumbnail->Add(App->resources->Reference((ResourceContainer*)editingMaerial));

				editingMaerial = new RE_Material();
				matName = "New Material";
			}
		}

		if (exits && !secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::Separator();
		editingMaerial->DrawMaterialEdit();

		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}

	ImGui::End();
}

SkyBoxEditorWindow::SkyBoxEditorWindow(const char* name, bool start_active) : EditorWindow(name, start_active)
{
	editingSkybox = new RE_SkyBox();
	sbName = "New Skybox";
}

SkyBoxEditorWindow::~SkyBoxEditorWindow()
{
	DEL(editingSkybox);
	if (previewImage != 0) glDeleteTextures(1, &previewImage);
}

void SkyBoxEditorWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		ImGui::Text("Skybox name:");
		ImGui::SameLine();
		ImGui::InputText("##sbname", &sbName);

		assetPath = "Assets/Skyboxes/";
		assetPath += sbName;
		assetPath += ".pupil";
		ImGui::Text("Save path: %s", assetPath.c_str());

		bool isTexturesFilled = editingSkybox->isFacesFilled();
		bool exits = App->fs->Exists(assetPath.c_str());
		if (exits) ImGui::Text("This skybox exits, change the name.");

		if (isTexturesFilled) ImGui::Text("Needed set all textures before save.");

		if ((exits || !isTexturesFilled) && !secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		ImGui::SameLine();
		if (ImGui::Button("Save")) {
			if (!sbName.empty() && !exits) {

				editingSkybox->SetName(sbName.c_str());
				editingSkybox->SetAssetPath(assetPath.c_str());
				editingSkybox->SetType(Resource_Type::R_SKYBOX);
				editingSkybox->AssetSave();
				editingSkybox->SaveMeta();

				App->thumbnail->Add(App->resources->Reference((ResourceContainer*)editingSkybox));

				editingSkybox = new RE_SkyBox();
				sbName = "New Skybox";
			}
		}

		if ((exits || !isTexturesFilled) && !secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::Separator();
		editingSkybox->DrawEditSkyBox();
	}

	ImGui::End();
}



ShaderEditorWindow::ShaderEditorWindow(const char* name, bool start_active) : EditorWindow(name, start_active)
{
	editingShader = new RE_Shader();
	shaderName = "New Shader";
	vertexPath = "";
	fragmentPath = "";
	geometryPath = "";
}

ShaderEditorWindow::~ShaderEditorWindow()
{
	DEL(editingShader);
}

void ShaderEditorWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		ImGui::Text("Shader name:");
		ImGui::SameLine();
		ImGui::InputText("##shadername", &shaderName);

		assetPath = "Assets/Shaders/";
		assetPath += shaderName;
		assetPath += ".meta";
		ImGui::Text("Save path: %s", assetPath.c_str());

		static bool compilePass = false;
		static bool vertexModify = false;
		static bool fragmentModify = false;
		static bool geometryModify = false;

		bool exits = App->fs->Exists(assetPath.c_str());
		if (exits) ImGui::Text("This shader exits, change the name.");
		bool neededVertexAndFragment = (vertexPath.empty() || fragmentPath.empty());
		if (neededVertexAndFragment) ImGui::Text("The vertex or fragment file path is empty.");

		static std::string* waitingPath = nullptr;
		if (waitingPath) ImGui::Text("Select Shader file from assets panel");

		bool pop2 = false;
		if (!compilePass) {
			ImGui::Text("Nedded pass the compile test.");
			pop2 = true;
		}
		if ((neededVertexAndFragment || exits || pop2) && !secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		ImGui::SameLine();
		if (ImGui::Button("Save")) {
			if (!shaderName.empty() && !exits) {
				editingShader->SetName(shaderName.c_str());
				editingShader->SetType(Resource_Type::R_SHADER);
				editingShader->SetPaths(vertexPath.c_str(), fragmentPath.c_str(), (!geometryPath.empty()) ? geometryPath.c_str() : nullptr);
				editingShader->isShaderFilesChanged();
				editingShader->SaveMeta();
				App->resources->Reference((ResourceContainer*)editingShader);

				editingShader = new RE_Shader();
				shaderName = "New Shader";
				vertexPath = "";
				fragmentPath = "";
				geometryPath = "";
				compilePass = false;
			}
		}

		if ((neededVertexAndFragment || exits || pop2) && !secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::Separator();
		ImGui::Text("The files that contains the script is an undefined file.");
		ImGui::Text("Select the script type and undefined file on panel assets.");
		ImGui::Text("When activate the selection, the undefied files from assets panel can be selected.");

		bool pop = (waitingPath);
		if (pop && !secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		ImGui::Separator();
		ImGui::Text("Vertex Shader Path:\n%s", (vertexPath.empty()) ? "No path." : vertexPath.c_str());
		if (ImGui::Button("Select vertex path")) {
			vertexPath.clear();
			waitingPath = &vertexPath;
			App->editor->SelectUndefinedFile(waitingPath);
			compilePass = false;
		}
		ImGui::SameLine();
		if (!vertexPath.empty() && !vertexModify) {
			if (ImGui::Button("Edit Vertex Shader")) {
				vertexModify = true;
				App->editor->OpenTextEditor(vertexPath.c_str(), &vertexPath, nullptr, &vertexModify);
			}

			ImGui::SameLine();
			if (ImGui::Button("Clear vertex")) vertexPath.clear();
		}
		else if (vertexPath.empty() && !vertexModify && !fragmentModify && !geometryModify) {
			if (ImGui::Button("New vertex shader")) {
				vertexModify = true;
				App->editor->OpenTextEditor(nullptr, &vertexPath, DEFVERTEXSHADER, &vertexModify);
			}
		}

		ImGui::Separator();
		ImGui::Text("Fragment Shader Path:\n%s", (fragmentPath.empty()) ? "No path." : fragmentPath.c_str());
		if (ImGui::Button("Select fragment path")) {
			fragmentPath.clear();
			waitingPath = &fragmentPath;
			App->editor->SelectUndefinedFile(waitingPath);
			compilePass = false;
		}
		ImGui::SameLine();
		if (!fragmentPath.empty() && !fragmentModify) {
			if (ImGui::Button("Edit Fragment Shader")) {
				fragmentModify = true;
				App->editor->OpenTextEditor(fragmentPath.c_str(), &fragmentPath, nullptr, &fragmentModify);
			}

			ImGui::SameLine();
			if (ImGui::Button("Clear fragment")) fragmentPath.clear();
		}
		else if (fragmentPath.empty() && !vertexModify && !fragmentModify && !geometryModify) {
			if (ImGui::Button("New fragment shader")) {
				fragmentModify = true;
				App->editor->OpenTextEditor(nullptr, &fragmentPath, DEFFRAGMENTSHADER, &fragmentModify);
			}
		}

		ImGui::Separator();
		ImGui::Text("Geometry Shader Path:\n%s", (geometryPath.empty()) ? "No path." : geometryPath.c_str());
		if (ImGui::Button("Select geometry path")) {
			geometryPath.clear();
			waitingPath = &geometryPath;
			App->editor->SelectUndefinedFile(waitingPath);
			compilePass = false;
		}
		ImGui::SameLine();
		if (!geometryPath.empty() && !geometryModify) {
			if (ImGui::Button("Edit Geometry Shader")) {
				geometryModify = true;
				App->editor->OpenTextEditor(geometryPath.c_str(), &geometryPath, nullptr, &geometryModify);
			}

			ImGui::SameLine();
			if (ImGui::Button("Clear geometry")) geometryPath.clear();
		}
		else if (geometryPath.empty() && !vertexModify && !fragmentModify && !geometryModify) {
			if (ImGui::Button("New geometry shader")) {
				geometryModify = true;
				App->editor->OpenTextEditor(nullptr, &geometryPath, nullptr, &geometryModify);
			}
		}

		ImGui::Separator();
		if (neededVertexAndFragment) ImGui::Text("The vertex or fragment file path is empty.");

		if (neededVertexAndFragment && !pop && !secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (!compilePass && ImGui::Button("Compile Test")) {
			App->handlerrors->StartHandling();

			uint sID = 0;
			if (App->shaders->LoadFromAssets(&sID, vertexPath.c_str(), fragmentPath.c_str(), (!geometryPath.empty()) ? geometryPath.c_str() : nullptr, true))
				compilePass = true;
			else
				LOG_ERROR("Shader Compilation Error:\n%s", App->shaders->GetShaderError());

			App->handlerrors->StopHandling();
			if (App->handlerrors->AnyErrorHandled())
				App->handlerrors->ActivatePopUp();
		}

		if (secondary || pop || neededVertexAndFragment) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		if (waitingPath) waitingPath = nullptr;
	}

	ImGui::End();
}

TextEditorManagerWindow::TextEditorManagerWindow(const char* name, bool start_active) : EditorWindow(name, start_active) {}

TextEditorManagerWindow::~TextEditorManagerWindow()
{
	for (auto e : editors) {
		if (e->file) DEL(e->file);
		if (e->textEditor) DEL(e->textEditor);
		DEL(e);
	}
}

void TextEditorManagerWindow::PushEditor(const char* filePath, std::string* newFile, const char* shadertTemplate, bool* open)
{
	for (auto e : editors) if (strcmp(e->toModify->c_str(), filePath) == 0) return;

	editor* e = new editor();
	if (filePath) {
		RE_FileIO* file = new RE_FileIO(filePath, App->fs->GetZipPath());

		if (file->Load()) {
			e->textEditor = new TextEditor();
			e->toModify = newFile;
			e->file = file;
			e->textEditor->SetText(file->GetBuffer());
		}
	}
	else {
		e->textEditor = new TextEditor();
		e->toModify = newFile;
		if (shadertTemplate) {
			e->textEditor->SetText(shadertTemplate);
			e->save = true;
		}
	}
	if (e->textEditor) editors.push_back(e);
	else {
		if (filePath) DEL(e->file);
		DEL(e);
	}

	if (e)e->open = open;
}

void TextEditorManagerWindow::Draw(bool secondary)
{
	static const char* compileAsStr[4] = { "None", "Vertex", "Fragment", "Geometry" };
	static std::vector<editor*> toRemoveE;
	static std::string assetPath;
	static std::string names;
	int count = -1;
	for (auto e : editors) {
		count++;
		bool close = false;
		names = "Text Editor #";
		names += std::to_string(count);
		if (ImGui::Begin(names.c_str(), 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
		{
			if (secondary) {
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}
			bool pop = false;
			if (!e->file) {
				ImGui::Text("Shader name:");
				ImGui::SameLine();
				ImGui::InputText("##newshadername", e->toModify);

				assetPath = "Assets/Shaders/";
				assetPath += *e->toModify;
				ImGui::Text("Save path: %s", assetPath.c_str());

				bool exits = App->fs->Exists(assetPath.c_str());
				if (pop = exits) ImGui::Text("This shader exits, change the name.");
			}
			else
				ImGui::Text("Editting %s", e->toModify->c_str());

			names = "Compile as shader script #";
			names += std::to_string(count);
			if (ImGui::Button(names.c_str())) {
				App->handlerrors->StartHandling();

				std::string text = e->textEditor->GetText();

				if (!(e->works = App->shaders->Compile(text.c_str(), text.size())))
					LOG_ERROR("%s", App->shaders->GetShaderError());

				e->compiled = true;

				App->handlerrors->StopHandling();
				if (App->handlerrors->AnyErrorHandled())
					App->handlerrors->ActivatePopUp();
			}

			if (e->compiled) ImGui::Text((e->works) ? "Succeful compile" : "Error compile");

			if (pop && !secondary) {
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (e->save) {
				ImGui::Text("Be sure to save before close!");
				names = "Save #";
				names += std::to_string(count);
				if (ImGui::Button(names.c_str())) {
					std::string text = e->textEditor->GetText();
					if (!e->file) {
						e->file = new RE_FileIO(assetPath.c_str(), App->fs->GetZipPath());
						*e->toModify = assetPath;
					}
					e->file->Save((char*)text.c_str(), text.size());

					e->save = false;
				}
				ImGui::SameLine();
			}

			if (pop && !secondary) {
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}

			names = "Quit #";
			names += std::to_string(count);
			if (ImGui::Button(names.c_str())) {
				close = true;
			}

			names = "Shader Text Editor #";
			names += std::to_string(count);
			e->textEditor->Render(names.c_str());

			if (secondary) {
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}

		ImGui::End();

		if (e->textEditor->IsTextChanged()) {
			e->compiled = false;
			e->save = true;
		}

		if (close) {
			if (!e->file) e->toModify->clear();
			if (e->open) *e->open = false;
			DEL(e->textEditor);
			if (e->file) DEL(e->file);
			toRemoveE.push_back(e);
		}
	}

	if (!toRemoveE.empty()) {
		//https://stackoverflow.com/questions/21195217/elegant-way-to-remove-all-elements-of-a-vector-that-are-contained-in-another-vec
		editors.erase(std::remove_if(std::begin(editors), std::end(editors),
			[&](auto x) {return std::find(begin(toRemoveE), end(toRemoveE), x) != end(toRemoveE); }), std::end(editors));
		for (auto e : toRemoveE) DEL(e);
		toRemoveE.clear();
	}
}
