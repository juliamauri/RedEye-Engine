#include "EditorWindows.h"

#include "Globals.h"
#include "RE_FileSystem.h"
#include "Application.h"
#include "ModuleEditor.h"
#include "ModuleScene.h"
#include "ModulePhysics.h"
#include "ModuleRenderer3D.h"
#include "ModuleInput.h"
#include "RE_PrimitiveManager.h"
#include "RE_FileBuffer.h"
#include "RE_ConsoleLog.h"
#include "RE_GLCache.h"
#include "RE_CameraManager.h"

#include "RE_DefaultShaders.h"
#include "RE_ShaderImporter.h"
#include "RE_ResourceManager.h"
#include "RE_InternalResources.h"
#include "RE_Material.h"
#include "RE_SkyBox.h"
#include "RE_Shader.h"
#include "RE_Prefab.h"
#include "RE_GameObject.h"
#include "RE_Particle.h"

//#include "RE_ParticleEmitter.h"

#include "ImGui/misc/cpp/imgui_stdlib.h"
#include "ImGuiWidgets/ImGuiColorTextEdit/TextEditor.h"

#define IMGUICURVEIMPLEMENTATION
#include "ImGuiWidgets/ImGuiCurverEditor/ImGuiCurveEditor.hpp"


#include "Glew/include/glew.h"

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
			editingMaerial->SetName(matName.c_str());
			editingMaerial->SetAssetPath(assetPath.c_str());
			editingMaerial->SetType(Resource_Type::R_MATERIAL);
			editingMaerial->Save();

			RE_RENDER->PushThumnailRend(RE_RES->Reference((ResourceContainer*)editingMaerial));

			editingMaerial = new RE_Material();
			matName = "New Material";
		}

		if (exits && !secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		ImGui::Separator();
		editingMaerial->DrawMaterialEdit();

		if (secondary)
		{
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
				editingShader->SetType(Resource_Type::R_SHADER);
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
			RE_LOGGER.ScopeProcedureLogging();

			uint sID = 0;
			compilePass = RE_ShaderImporter::LoadFromAssets(&sID, vertexPath.c_str(), fragmentPath.c_str(), (!geometryPath.empty()) ? geometryPath.c_str() : nullptr, true);
			if (!compilePass) RE_LOG_ERROR("Shader Compilation Error:\n%s", RE_ShaderImporter::GetShaderError());

			RE_LOGGER.EndScope();
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

TextEditorManagerWindow::TextEditorManagerWindow(const char* name, bool start_active) : EditorWindow(name, start_active) {}

TextEditorManagerWindow::~TextEditorManagerWindow()
{
	for (auto e : editors)
	{
		if (e->file) DEL(e->file);
		if (e->textEditor) DEL(e->textEditor);
		DEL(e);
	}
}

void TextEditorManagerWindow::PushEditor(const char* filePath, eastl::string* newFile, const char* shadertTemplate, bool* open)
{
	for (auto e : editors)
		if (strcmp(e->toModify->c_str(), filePath) == 0)
			return;

	editor* e = new editor();
	if (filePath)
	{
		RE_FileBuffer* file = new RE_FileBuffer(filePath, RE_FS->GetZipPath());
		if (file->Load())
		{
			e->textEditor = new TextEditor();
			e->toModify = newFile;
			e->file = file;
			e->textEditor->SetText(file->GetBuffer());
		}
	}
	else
	{
		e->textEditor = new TextEditor();
		e->toModify = newFile;
		if (shadertTemplate)
		{
			e->textEditor->SetText(shadertTemplate);
			e->save = true;
		}
	}

	if (e->textEditor)
	{
		editors.push_back(e);
	}
	else
	{
		if (filePath) DEL(e->file);
		DEL(e);
	}

	if (e)e->open = open;
}

void TextEditorManagerWindow::Draw(bool secondary)
{
	static const char* compileAsStr[4] = { "None", "Vertex", "Fragment", "Geometry" };
	static eastl::vector<editor*> toRemoveE;
	static eastl::string assetPath;
	static eastl::string names;
	int count = -1;
	for (auto e : editors)
	{
		count++;
		bool close = false;
		names = "Text Editor #";
		names += eastl::to_string(count);
		if (ImGui::Begin(names.c_str(), 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			bool pop = false, nameReserved = false;
			if (!e->file)
			{
				ImGui::Text("Shader name:");
				ImGui::SameLine();
				ImGui::InputText("##newshadername", e->toModify);
				(assetPath = "Assets/Shaders/") += *e->toModify;
				ImGui::Text("Save path: %s", assetPath.c_str());
				if (pop = RE_FS->Exists(assetPath.c_str())) ImGui::Text("This shader exits, change the name.");

				if (*e->toModify == "Water.vert" || *e->toModify == "Water.frag" || *e->toModify == "WaterDeferred.vert" || *e->toModify == "WaterDeferred.frag") {
					nameReserved = true;
					ImGui::Text("This name is reserved!");
				}
			}
			else
				ImGui::Text("Editting %s", e->toModify->c_str());

			names = "Compile as shader script #" + eastl::to_string(count);
			if (ImGui::Button(names.c_str()))
			{
				RE_LOGGER.ScopeProcedureLogging();

				std::string tmp = e->textEditor->GetText();
				eastl::string text(tmp.c_str(), tmp.size());
				e->works = RE_ShaderImporter::Compile(text.c_str(), text.size());
				if (!e->works) RE_LOG_ERROR("%s", RE_ShaderImporter::GetShaderError());
				e->compiled = true;

				RE_LOGGER.EndScope();
			}

			if (e->compiled) ImGui::Text((e->works) ? "Succeful compile" : "Error compile");

			if ((nameReserved || pop) && !secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (e->save)
			{
				ImGui::Text("Be sure to save before close!");
				names = "Save #";
				names += eastl::to_string(count);

				if (ImGui::Button(names.c_str()))
				{
					eastl::string text = e->textEditor->GetTextPtr();
					if (!e->file)
					{
						e->file = new RE_FileBuffer(assetPath.c_str(), RE_FS->GetZipPath());
						*e->toModify = assetPath;
					}

					e->file->Save((char*)text.c_str(), text.size());
					e->save = false;
				}

				ImGui::SameLine();
			}

			if ((nameReserved || pop) && !secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}

			names = "Quit #" + eastl::to_string(count);
			if (ImGui::Button(names.c_str())) close = true;

			(names = "Shader Text Editor #") += eastl::to_string(count);
			e->textEditor->Render(names.c_str());

			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}

		ImGui::End();

		if (e->textEditor->IsTextChanged())
		{
			e->compiled = false;
			e->save = true;
		}

		if (close)
		{
			if (!e->file) e->toModify->clear();
			if (e->open) *e->open = false;
			DEL(e->textEditor);
			if (e->file) DEL(e->file);
			toRemoveE.push_back(e);
		}
	}

	if (!toRemoveE.empty())
	{
		//https://stackoverflow.com/questions/21195217/elegant-way-to-remove-all-elements-of-a-vector-that-are-contained-in-another-vec
		editors.erase(eastl::remove_if(eastl::begin(editors), eastl::end(editors),
			[&](auto x) {return eastl::find(begin(toRemoveE), end(toRemoveE), x) != end(toRemoveE); }), eastl::end(editors));
		for (auto e : toRemoveE) DEL(e);
		toRemoveE.clear();
	}
}

WaterPlaneResourceWindow::WaterPlaneResourceWindow(const char* name, bool start_active) : EditorWindow(name, start_active), waterResouceName("WaterMaterial") {}

WaterPlaneResourceWindow::~WaterPlaneResourceWindow() { }

void WaterPlaneResourceWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		ImGui::TextWrapped("After generate resource, create a primitive plane, set slices and stacks and finally transform as mesh, then you can add the new material.\n\
			Remember to set the values and save on the material generated. Important to set direction, not 0 - 0.");

		ImGui::Checkbox("Deferred", &deferred);

		eastl::string shaderPath("Assets/Shaders/");

		shaderPath += (deferred) ? "WaterDeferredShader" : "WaterShader";
		shaderPath += ".meta";

		const char* waterShader = RE_RES->FindMD5ByMETAPath(shaderPath.c_str(), R_SHADER);
		if (!waterShader) {
			ImGui::Text((deferred) ? "Water Deferred Shader doesn't exists." : "Water Shader doesn't exists.");
			ImGui::Text("Shader will generate when create.");
		}
		else ImGui::Text("Shader detected.");

		eastl::string materialPath("Assets/Materials/");
		ImGui::InputText("#WaterMaterialName", &waterResouceName);
		materialPath += waterResouceName;
		materialPath += ".pupil";

		bool exists = RE_FS->Exists(materialPath.c_str());
		if (exists) ImGui::Text("That Material exists!");

		if (exists && !secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (ImGui::Button("Create water resource")) {

			if (!waterShader) {
				eastl::string shaderVertexFile("Assets/Shaders/");
				shaderVertexFile += (deferred) ? "WaterDeferred.vert" : "Water.vert";
				if (!RE_FS->Exists(shaderVertexFile.c_str())) {
					RE_FileBuffer vertexFile(shaderVertexFile.c_str(), RE_FS->GetZipPath());
					vertexFile.Save((deferred) ? WATERPASSVERTEXSHADER : WATERVERTEXSHADER, 
						eastl::CharStrlen((deferred) ? WATERPASSVERTEXSHADER : WATERVERTEXSHADER));
				}

				eastl::string shaderFragmentFile("Assets/Shaders/");
				shaderFragmentFile += (deferred) ? "WaterDeferred.frag" : "Water.frag";
				if (!RE_FS->Exists(shaderFragmentFile.c_str())) {
					RE_FileBuffer fragmentFile(shaderFragmentFile.c_str(), RE_FS->GetZipPath());
					fragmentFile.Save((deferred) ? WATERPASSFRAGMENTSHADER : WATERFRAGMENTSHADER,
						eastl::CharStrlen((deferred) ? WATERPASSFRAGMENTSHADER : WATERFRAGMENTSHADER));
				}
				RE_Shader* waterShaderRes = new RE_Shader();
				waterShaderRes->SetName((deferred) ? "WaterDeferredShader" : "WaterShader");
				waterShaderRes->SetType(Resource_Type::R_SHADER);
				waterShaderRes->SetMetaPath(shaderPath.c_str());
				waterShaderRes->SetPaths(shaderVertexFile.c_str(), shaderFragmentFile.c_str(), nullptr);
				waterShaderRes->isShaderFilesChanged();
				waterShaderRes->SaveMeta();
				waterShader = RE_RES->Reference(static_cast<ResourceContainer*>(waterShaderRes));
			}

			RE_Material* editingMaterialRes = new RE_Material();
			editingMaterialRes->blendMode = true;
			editingMaterialRes->SetName(waterResouceName.c_str());
			editingMaterialRes->SetAssetPath(materialPath.c_str());
			editingMaterialRes->SetType(Resource_Type::R_MATERIAL);
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


ParticleEmiiterEditorWindow::ParticleEmiiterEditorWindow(const char* name, bool start_active) : EditorWindow(name, start_active) {}

ParticleEmiiterEditorWindow::~ParticleEmiiterEditorWindow() {}

void ParticleEmiiterEditorWindow::StartEditing(RE_ParticleEmitter* sim, UID cmp)
{
	active = true;
	simulation = sim;
	fromComponent = cmp;
}

UID ParticleEmiiterEditorWindow::GetComponent() const
{
	return fromComponent;
}

void ParticleEmiiterEditorWindow::UpdateViewPort()
{
	RE_CameraManager::ParticleEditorCamera()->GetTargetViewPort(viewport);
	viewport.x = (width - viewport.z) * 0.5f;
	viewport.y = (heigth - viewport.w) * 0.5f + 20;
}

void ParticleEmiiterEditorWindow::Recalc() { recalc = true; }

void ParticleEmiiterEditorWindow::Draw(bool secondary)
{
	if (simulation != nullptr)
	{
		if(!docking) ImGui::GetIO().ConfigFlags -= ImGuiConfigFlags_DockingEnable;

		ImGuiWindowFlags wFlags = ImGuiWindowFlags_::ImGuiWindowFlags_None;
		wFlags |= ImGuiWindowFlags_NoCollapse;// | ImGuiWindowFlags_NoTitleBar;

		if (ImGui::Begin("Particle Controller", &active, wFlags | ImGuiWindowFlags_MenuBar))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (ImGui::BeginMenuBar())
				ImGui::Checkbox(!docking ? "Enable Docking" : "Disable Docking", &docking);
			ImGui::EndMenuBar();

			switch (simulation->state)
			{
			case RE_ParticleEmitter::STOP:
			{
				if (ImGui::Button("Play")) simulation->state = RE_ParticleEmitter::PLAY;
				break;
			}
			case RE_ParticleEmitter::PLAY:
			{
				if (ImGui::Button("Pause")) simulation->state = RE_ParticleEmitter::PAUSE;
				ImGui::SameLine();
				if (ImGui::Button("Stop")) simulation->state = RE_ParticleEmitter::STOP;
				break;
			}
			case RE_ParticleEmitter::PAUSE:
			{
				if (ImGui::Button("Resume")) simulation->state = RE_ParticleEmitter::PLAY;
				ImGui::SameLine();
				if (ImGui::Button("Stop")) simulation->state = RE_ParticleEmitter::STOP;
				break;
			}
			}

			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		if (ImGui::Begin(name, &active, wFlags))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			static int lastWidht = 0;
			static int lastHeight = 0;
			ImVec2 size = ImGui::GetWindowSize();
			width = static_cast<int>(size.x);
			heigth = static_cast<int>(size.y) - 28;
			if (recalc || lastWidht != width || lastHeight != heigth)
			{
				RE_INPUT->Push(RE_EventType::PARTRICLEEDITORWINDOWCHANGED, RE_RENDER, RE_Cvar(lastWidht = width), RE_Cvar(lastHeight = heigth));
				RE_INPUT->Push(RE_EventType::PARTRICLEEDITORWINDOWCHANGED, RE_EDITOR);
				recalc = false;
			}

			isWindowSelected = (ImGui::IsWindowHovered() && ImGui::IsWindowFocused(ImGuiHoveredFlags_AnyWindow));
			ImGui::SetCursorPos({ viewport.x, viewport.y });
			ImGui::Image((void*)RE_RENDER->GetRenderedParticleEditorTexture(), { viewport.z, viewport.w }, { 0.0, 1.0 }, { 1.0, 0.0 });

			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Particle Settings", &active, wFlags))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			ImGui::Text("Current particles: %i", simulation->particle_count);

			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Particle Renderer Settings", &active, wFlags))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (ImGui::DragFloat3("Scale", simulation->scale.ptr(), 0.1f, -10000.f, 10000.f, "%.2f")) {
				if (!simulation->scale.IsFinite())simulation->scale.Set(0.5f, 0.5f, 0.5f);
			}

			int pDir = simulation->particleDir;
			if (ImGui::Combo("Particle Direction", &pDir, "Normal\0Billboard\0Custom\0"))
				simulation->particleDir = static_cast<RE_ParticleEmitter::Particle_Dir>(pDir);

			if (simulation->particleDir == RE_ParticleEmitter::PS_Custom)
				ImGui::DragFloat3("Custom Direction", simulation->direction.ptr(), 0.1f, -1.f, 1.f, "%.2f");


			int cState = simulation->cState;
			if (ImGui::Combo("Color Type", &cState, "Single\0Over Lifetime\0Over Distance\0Over Speed\0"))
				simulation->cState = static_cast<RE_ParticleEmitter::ColorState>(cState);

			switch (simulation->cState)
			{
			case RE_ParticleEmitter::ColorState::SINGLE:
				ImGui::ColorEdit3("Particle Color", &simulation->particleColor[0]);
				break;
			default:
				ImGui::ColorEdit3("Particle Gradient 1", &simulation->gradient[0][0]);
				ImGui::ColorEdit3("Particle Gradient 2", &simulation->gradient[1][0]);
				break;
			}

			ImGui::Checkbox("Use Opacity", &simulation->useOpacity);
			if (!simulation->opacityWithCurve) {
				ImGui::SameLine();
				ImGui::PushItemWidth(200.f);
				ImGui::SliderFloat("Opacity", &simulation->opacity, 0.0f, 1.0f);
				ImGui::PopItemWidth();
			}
			else
				ImGui::Text("Opacity is with curve");

			if (!simulation->materialMD5) ImGui::Text("NMaterial not selected.");
			else
			{
				ImGui::Separator();
				RE_Material* matRes = dynamic_cast<RE_Material*>(RE_RES->At(simulation->materialMD5));
				if (ImGui::Button(matRes->GetName())) RE_RES->PushSelected(matRes->GetMD5(), true);

				matRes->DrawMaterialParticleEdit(simulation->useTextures);
			}

			ImGui::Separator();

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
						if (simulation->materialMD5)
						{
							(dynamic_cast<RE_Material*>(RE_RES->At(simulation->materialMD5)))->UnUseResources();
							RE_RES->UnUse(simulation->materialMD5);
						}
						simulation->materialMD5 = material->GetMD5();
						if (simulation->materialMD5)
						{
							RE_RES->Use(simulation->materialMD5);
							(dynamic_cast<RE_Material*>(RE_RES->At(simulation->materialMD5)))->UseResources();
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
					if (simulation->materialMD5) RE_RES->UnUse(simulation->materialMD5);
					simulation->materialMD5 = *static_cast<const char**>(dropped->Data);
					if (simulation->materialMD5) RE_RES->Use(simulation->materialMD5);
				}
				ImGui::EndDragDropTarget();
			}


			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Particle Form", &active, wFlags))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (simulation->meshMD5)
			{
				if (ImGui::Button(eastl::string("Resource Mesh").c_str()))
					RE_RES->PushSelected(simulation->meshMD5, true);
			}
			else if (simulation->primCmp)
			{
				simulation->primCmp->DrawPrimPropierties();
			}
			else ImGui::TextWrapped("Select mesh resource or select primitive");

			ImGui::Separator();

			static bool clearMesh = false, setUpPrimitive = false;
			if (ImGui::BeginMenu("Primitive"))
			{
				if (ImGui::MenuItem("Point")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompPoint();
					setUpPrimitive = clearMesh = true;
					simulation->useTextures = false;
				}
				if (ImGui::MenuItem("Cube")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompCube();
					setUpPrimitive = clearMesh = true;
					simulation->useTextures = false;
				}
				if (ImGui::MenuItem("Dodecahedron")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompDodecahedron();
					setUpPrimitive = clearMesh = true;
					simulation->useTextures = false;
				}
				if (ImGui::MenuItem("Tetrahedron")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompTetrahedron();
					setUpPrimitive = clearMesh = true;
					simulation->useTextures = false;
				}
				if (ImGui::MenuItem("Octohedron")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompOctohedron();
					setUpPrimitive = clearMesh = true;
					simulation->useTextures = false;
				}
				if (ImGui::MenuItem("Icosahedron")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompIcosahedron();
					setUpPrimitive = clearMesh = true;
					simulation->useTextures = false;
				}
				if (ImGui::MenuItem("Plane")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompPlane();
					simulation->useTextures = setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Sphere")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompSphere();
					simulation->useTextures = setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Cylinder")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompCylinder();
					simulation->useTextures = setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("HemiSphere")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompHemiSphere();
					simulation->useTextures = setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Torus")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompTorus();
					simulation->useTextures = setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Trefoil Knot")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompTrefoiKnot();
					simulation->useTextures = setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Rock")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompRock();
					setUpPrimitive = clearMesh = true;
					simulation->useTextures = false;
				}

				ImGui::EndMenu();
			}

			if (clearMesh)
			{
				if (simulation->meshMD5) {
					RE_RES->UnUse(simulation->meshMD5);
					simulation->meshMD5 = nullptr;
				}

				if (setUpPrimitive) {
					RE_SCENE->primitives->SetUpComponentPrimitive(simulation->primCmp);
					setUpPrimitive = false;
				}

				clearMesh = false;
			}

			if (ImGui::BeginMenu("Change mesh"))
			{
				eastl::vector<ResourceContainer*> meshes = RE_RES->GetResourcesByType(Resource_Type::R_MESH);
				bool none = true;
				unsigned int count = 0;
				for (auto m : meshes)
				{
					if (m->isInternal()) continue;

					none = false;
					eastl::string name = eastl::to_string(count++) + m->GetName();
					if (ImGui::MenuItem(name.c_str()))
					{
						if (simulation->meshMD5) RE_RES->UnUse(simulation->meshMD5);
						simulation->meshMD5 = m->GetMD5();
						if (simulation->meshMD5) RE_RES->Use(simulation->meshMD5);

						simulation->useTextures = true;
					}
				}
				if (none) ImGui::Text("No custom materials on assets");

				ImGui::EndMenu();
			}
			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Particle Lighting", &active, wFlags))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}


			ImGui::Checkbox(simulation->emitlight ? "Disable lighting" : "Enable lighting", & simulation->emitlight);
			ImGui::ColorEdit3("Light Color", &simulation->lightColor[0]);
			if (simulation->particleLColor) {
				if (ImGui::Button("Set particles light color")) {
					eastl::list<RE_Particle*>* particles = RE_PHYSICS->GetParticles(simulation->id);
					for (auto p : *particles) p->lightColor = simulation->lightColor;
				}
			}
			ImGui::DragFloat("Specular", &simulation->specular, 0.01f, 0.f, 1.f, "%.2f");
			if (simulation->particleLColor) {
				if (ImGui::Button("Set particles specular")) {
					eastl::list<RE_Particle*>* particles = RE_PHYSICS->GetParticles(simulation->id);
					for (auto p : *particles) p->specular = simulation->specular;
				}
			}
			ImGui::DragFloat("Intensity", &simulation->intensity, 0.01f, 0.0f, 50.0f, "%.2f");
			if (simulation->particleLColor) {
				if (ImGui::Button("Set particles intensity")) {
					eastl::list<RE_Particle*>* particles = RE_PHYSICS->GetParticles(simulation->id);
					for (auto p : *particles) p->intensity = simulation->intensity;
				}
			}
			ImGui::DragFloat("Constant", &simulation->constant, 0.01f, 0.001f, 5.0f, "%.2f");
			ImGui::DragFloat("Linear", &simulation->linear, 0.001f, 0.001f, 5.0f, "%.3f");
			ImGui::DragFloat("Quadratic", &simulation->quadratic, 0.001f, 0.001f, 5.0f, "%.3f");
			ImGui::Separator();
			ImGui::Checkbox(simulation->particleLColor ? "Disable single particle lighting" : "Enable single particle lighting", &simulation->particleLColor);
			if (simulation->particleLColor) {
				if (ImGui::Checkbox(simulation->randomLightColor ? "Disable random particle color" : "Enable random particle color", &simulation->randomLightColor)) {

					eastl::list<RE_Particle*>* particles = RE_PHYSICS->GetParticles(simulation->id);
					if (simulation->randomLightColor)
						for (auto p : *particles) p->lightColor.Set(RE_MATH->RandomF(), RE_MATH->RandomF(), RE_MATH->RandomF());
					else
						for (auto p : *particles) p->lightColor = simulation->lightColor;
				}

				ImGui::DragFloat2("Specular min-max", simulation->sClamp, 0.005f, 0.0f, 1.0f);
				if (ImGui::Checkbox(simulation->randomSpecular ? "Disable random particle specular" : "Enable random particle specular", &simulation->randomSpecular)) {

					eastl::list<RE_Particle*>* particles = RE_PHYSICS->GetParticles(simulation->id);

					if (simulation->randomSpecular)
						for (auto p : *particles) p->specular = RE_MATH->RandomF(simulation->sClamp[0], simulation->sClamp[1]);
					else
						for (auto p : *particles) p->specular = simulation->specular;
				}

				ImGui::DragFloat2("Intensity min-max", simulation->iClamp, 0.1f, 0.0f, 50.0f);
				if (ImGui::Checkbox(simulation->randomIntensity ? "Disable random particle intensity" : "Enable random particle intensity", &simulation->randomIntensity)) {
					eastl::list<RE_Particle*>* particles = RE_PHYSICS->GetParticles(simulation->id);

					if (simulation->randomIntensity)
						for (auto p : *particles) p->intensity = RE_MATH->RandomF(simulation->iClamp[0], simulation->iClamp[1]);
					else
						for (auto p : *particles) p->intensity = simulation->intensity;
				}
			}


			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Particle Curve", &active, wFlags))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			ImGui::Checkbox("Use curve", &simulation->useCurve);

			if (simulation->useCurve) {

				static float cSize[2] = { 600.f, 200.f };


				ImGui::SameLine();

				ImGui::PushItemWidth(150.f);

				ImGui::DragFloat2("Curve size", cSize, 1.0f, 0.0f, 0.0f, "%.0f");

				ImGui::PopItemWidth();

				ImGui::Curve("Particle curve editor", { cSize[0], cSize[1] }, simulation->total_points, simulation->curve.data());

				ImGui::SameLine();

				ImGui::PushItemWidth(50.f);

				static int minPoitns = 3;

				if (ImGui::DragInt("Num Points", &simulation->total_points, 1.0f)) {

					if (simulation->total_points < minPoitns) simulation->total_points = minPoitns;

					simulation->curve.clear();
					simulation->curve.push_back({ -1.0f, 0.0f });
					for (int i = 1; i < simulation->total_points; i++)
						simulation->curve.push_back({ 0.0f, 0.0f });
				}

				ImGui::SameLine();


				ImGui::Checkbox("Smooth curve", &simulation->smoothCurve);

				ImGui::SameLine();

				ImGui::Checkbox("Opacity with curve", &simulation->opacityWithCurve);

			}
			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	}
}
