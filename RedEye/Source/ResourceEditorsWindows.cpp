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
#include "RE_ParticleEmitterBase.h"
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

void ParticleEmiiterEditorWindow::StartEditing(RE_ParticleEmitter* sim, const char* md5)
{
	if (emiter_md5 != md5 || md5 == nullptr) {
		if (active || simulation || emiter_md5) {
			if (need_save) {
				next_emiter_md5 = md5;
				next_simulation = sim;
				load_next = true;

				if (emiter_md5) {
					RE_ParticleEmitterBase* emitter = dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emiter_md5));
					bool has_emissor = emitter->HasEmissor(), has_render = emitter->HasRenderer();
					RE_EDITOR->popupWindow->PopUpSaveParticles((!has_emissor || !has_render), true, has_emissor, has_render);
				}
				else RE_EDITOR->popupWindow->PopUpSaveParticles(true, false, new_emitter->HasEmissor(), new_emitter->HasRenderer());

				return;
			}
			else CloseEditor();
		}
		emiter_md5 = md5;
		if (!emiter_md5) new_emitter = new RE_ParticleEmitterBase();
		active = true;
		simulation = sim;
		RE_PHYSICS->AddEmitter(simulation);
	}
}

void ParticleEmiiterEditorWindow::SaveEmitter(bool close, const char* emitter_name, const char* emissor_base, const char* renderer_base)
{
	if (emiter_md5)
	{
		RE_ParticleEmitterBase* emitter = dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emiter_md5));
		if(!emitter->HasEmissor() || !emitter->HasRenderer())
			emitter->GenerateSubResourcesAndReference(emissor_base, renderer_base);
		emitter->FillAndSave(simulation);
	}
	else {
		new_emitter->SetName(emitter_name);
		new_emitter->SetType(R_PARTICLE_EMITTER);
		new_emitter->GenerateSubResourcesAndReference(emissor_base, renderer_base);
		new_emitter->FillAndSave(simulation);

		emiter_md5 = RE_RES->Reference(dynamic_cast<ResourceContainer*>(new_emitter));
	}

	need_save = false;

	if (load_next) LoadNextEmitter();
	else if (close) CloseEditor();
}

void ParticleEmiiterEditorWindow::NextOrClose()
{
	if (load_next)LoadNextEmitter();
	else CloseEditor();
}

void ParticleEmiiterEditorWindow::CloseEditor()
{
	if (simulation) RE_PHYSICS->RemoveEmitter(simulation);
	simulation = nullptr;
	if (!emiter_md5 && new_emitter) DEL(new_emitter);
	if(emiter_md5) RE_RES->UnUse(emiter_md5);
	emiter_md5 = nullptr;
	new_emitter = nullptr;
	active = false;
}

void ParticleEmiiterEditorWindow::LoadNextEmitter()
{
	CloseEditor();

	emiter_md5 = next_emiter_md5;
	if (!emiter_md5) new_emitter = new RE_ParticleEmitterBase();
	active = true;
	simulation = next_simulation;
	RE_PHYSICS->AddEmitter(simulation);

	load_next = false;
	next_emiter_md5 = nullptr;
	simulation = nullptr;
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
		bool close = false;

		if(!docking) ImGui::GetIO().ConfigFlags -= ImGuiConfigFlags_DockingEnable;

		ImGuiWindowFlags wFlags = ImGuiWindowFlags_::ImGuiWindowFlags_None;
		wFlags |= ImGuiWindowFlags_NoCollapse;// | ImGuiWindowFlags_NoTitleBar;

		// Playback Controls
		if (ImGui::Begin("Playback Controls", NULL, wFlags | ImGuiWindowFlags_MenuBar))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (ImGui::BeginMenuBar())
			{
				// Playback
				switch (simulation->state)
				{
				case RE_ParticleEmitter::STOP:
				{
					if (ImGui::Button("Play simulation")) simulation->state = RE_ParticleEmitter::PLAY;
					break;
				}
				case RE_ParticleEmitter::PLAY:
				{
					if (ImGui::Button("Pause simulation")) simulation->state = RE_ParticleEmitter::PAUSE;
					ImGui::SameLine();
					if (ImGui::Button("Stop simulation")) simulation->state = RE_ParticleEmitter::STOPING;
					break;
				}
				case RE_ParticleEmitter::PAUSE:
				{
					if (ImGui::Button("Resume simulation")) simulation->state = RE_ParticleEmitter::PLAY;
					ImGui::SameLine();
					if (ImGui::Button("Stop simulation")) simulation->state = RE_ParticleEmitter::STOPING;
					break;
				}
				}

				ImGui::SameLine();
				ImGui::Checkbox(!docking ? "Enable Docking" : "Disable Docking", &docking);


				ImGui::SameLine();
				if (ImGui::BeginMenu("Change emissor"))
				{
					eastl::vector<ResourceContainer*> meshes = RE_RES->GetResourcesByType(Resource_Type::R_PARTICLE_EMISSION);
					bool none = true;
					unsigned int count = 0;
					for (auto m : meshes)
					{
						if (m->isInternal()) continue;

						none = false;
						eastl::string name = eastl::to_string(count++) + m->GetName();
						if (ImGui::MenuItem(name.c_str()))
						{
							if (emiter_md5) dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emiter_md5))->ChangeEmissor(simulation, m->GetMD5());
							else new_emitter->ChangeEmissor(simulation, m->GetMD5());

							need_save = true;
						}
					}
					if (none) ImGui::Text("No particle emissors on assets");

					ImGui::EndMenu();
				}

				ImGui::SameLine();

				if (ImGui::BeginMenu("Change render"))
				{
					eastl::vector<ResourceContainer*> meshes = RE_RES->GetResourcesByType(Resource_Type::R_PARTICLE_RENDER);
					bool none = true;
					unsigned int count = 0;
					for (auto m : meshes)
					{
						if (m->isInternal()) continue;

						none = false;
						eastl::string name = eastl::to_string(count++) + m->GetName();
						if (ImGui::MenuItem(name.c_str()))
						{
							if (emiter_md5) dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emiter_md5))->ChangeRenderer(simulation, m->GetMD5());
							else new_emitter->ChangeRenderer(simulation, m->GetMD5());

							need_save = true;
						}
					}
					if (none) ImGui::Text("No particle renders on assets");

					ImGui::EndMenu();
				}

				bool disabled = false;
				if (!need_save && !secondary) {
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
					disabled = true;
				}

				ImGui::SameLine();
				if (ImGui::Button("Save"))
				{
					if (emiter_md5) {
						RE_ParticleEmitterBase* emitter = dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emiter_md5));
						bool has_emissor = emitter->HasEmissor(), has_render = emitter->HasRenderer();
						RE_EDITOR->popupWindow->PopUpSaveParticles((!has_emissor || !has_render), true, has_emissor, has_render);
					}
					else RE_EDITOR->popupWindow->PopUpSaveParticles(true, false, new_emitter->HasEmissor(), new_emitter->HasRenderer());
				}
				ImGui::SameLine();
				if (ImGui::Button("Discard changes"))
				{
					DEL(simulation);
					simulation = (emiter_md5) ? dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emiter_md5))->GetNewEmitter() 
						: new RE_ParticleEmitter(true);

					if (!emiter_md5) {
						DEL(new_emitter);
						new_emitter = new RE_ParticleEmitterBase();
					}

					need_save = false;
				}

				if (disabled)
				{
					ImGui::PopItemFlag();
					ImGui::PopStyleVar();
				}

				ImGui::SameLine();

				if (ImGui::Button("Close")) close = true; 
			}
			ImGui::EndMenuBar();

			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		// Viewport
		if (ImGui::Begin(name, NULL, wFlags))
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

		if (ImGui::Begin("Status", NULL, wFlags))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			// Control (read-only)
			ImGui::Text("Current particles: %i", simulation->particle_count);
			ImGui::Text("Total time: %.1f s", simulation->total_time);
			ImGui::Text("Max Distance: %.1f units", math::SqrtFast(simulation->max_dist_sq));
			ImGui::Text("Max Speed: %.1f units/s", math::SqrtFast(simulation->max_speed_sq));
			ImGui::Text("Parent Position: %.1f, %.1f, %.1f", simulation->parent_pos.x, simulation->parent_pos.y, simulation->parent_pos.z);
			ImGui::Text("Parent Speed: %.1f, %.1f, %.1f", simulation->parent_speed.x, simulation->parent_speed.y, simulation->parent_speed.z);
			
			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Spawning", NULL, wFlags))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			int tmp = static_cast<int>(simulation->max_particles);
			if (ImGui::DragInt("Max particles", &tmp, 1.f, 0, 65000)) {
				simulation->max_particles = static_cast<unsigned int>(tmp);
				need_save = true;
			}
			
			if (simulation->spawn_interval.DrawEditor(need_save) + simulation->spawn_mode.DrawEditor(need_save))
				if (simulation->state != RE_ParticleEmitter::PlaybackState::STOP)
					simulation->state = RE_ParticleEmitter::PlaybackState::RESTART;

			ImGui::Separator();
			if (ImGui::Checkbox("Start on Play", &simulation->start_on_play))  need_save = true; 
			if (ImGui::Checkbox("Loop", &simulation->loop))  need_save = true; 
			if (!simulation->loop)
			{
				ImGui::SameLine();
				if(ImGui::DragFloat("Max time", &simulation->max_time, 1.f, 0.f, 10000.f)) {
				need_save = true;
			}
			}
			if(ImGui::DragFloat("Time Multiplier", &simulation->time_muliplier, 0.01f, 0.01f, 10.f)) need_save = true;
			if(ImGui::DragFloat("Start Delay", &simulation->start_delay, 1.f, 0.f, 10000.f)) need_save = true;

			ImGui::Separator();
			if(ImGui::Checkbox("Local Space", &simulation->local_space)) need_save = true;
			if(ImGui::Checkbox("Inherit Speed", &simulation->inherit_speed)) need_save = true;

			ImGui::Separator();
			if(simulation->initial_lifetime.DrawEditor("Lifetime")) need_save = true;

			ImGui::Separator();
			if(simulation->initial_pos.DrawEditor()) need_save = true;

			ImGui::Separator();
			if(simulation->initial_speed.DrawEditor("Starting Speed")) need_save = true;

			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Physics", NULL, wFlags))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if(simulation->external_acc.DrawEditor()) need_save = true;

			ImGui::Separator();
			if(simulation->boundary.DrawEditor()) need_save = true;

			ImGui::Separator();
			if(simulation->collider.DrawEditor()) need_save = true;

			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Particle Shape", NULL, wFlags))
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
				if(simulation->primCmp->DrawPrimPropierties()) need_save = true;
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
				}
				if (ImGui::MenuItem("Cube")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompCube();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Dodecahedron")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompDodecahedron();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Tetrahedron")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompTetrahedron();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Octohedron")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompOctohedron();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Icosahedron")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompIcosahedron();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Plane")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompPlane();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Sphere")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompSphere();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Cylinder")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompCylinder();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("HemiSphere")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompHemiSphere();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Torus")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompTorus();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Trefoil Knot")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompTrefoiKnot();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Rock")) {
					if (simulation->primCmp) {
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp);
					}
					simulation->primCmp = new RE_CompRock();
					setUpPrimitive = clearMesh = true;
				}

				ImGui::EndMenu();
			}

			if (clearMesh)
			{
				need_save = true;

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

						need_save = true;
					}
				}
				if (none) ImGui::Text("No meshes on assets");

				ImGui::EndMenu();
			}
			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Particle Lighting", NULL, wFlags))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if(simulation->light.DrawEditor(simulation->id)) need_save = true;

			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Particle Renderer Settings", NULL, wFlags))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (ImGui::DragFloat3("Scale", simulation->scale.ptr(), 0.1f, -10000.f, 10000.f, "%.2f")) {
				if (!simulation->scale.IsFinite())simulation->scale.Set(0.5f, 0.5f, 0.5f);
				need_save = true;
			}

			int pDir = simulation->particleDir;
			if (ImGui::Combo("Particle Direction", &pDir, "Normal\0Billboard\0Custom\0")) {
				simulation->particleDir = static_cast<RE_ParticleEmitter::Particle_Dir>(pDir);
				need_save = true;
			}

			if (simulation->particleDir == RE_ParticleEmitter::PS_Custom) {
				ImGui::DragFloat3("Custom Direction", simulation->direction.ptr(), 0.1f, -1.f, 1.f, "%.2f");
				need_save = true;
			}

			if(simulation->color.DrawEditor()) need_save = true;
			if(simulation->opacity.DrawEditor()) need_save = true;

			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Opacity Curve", NULL, wFlags))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (simulation->opacity.type != RE_PR_Opacity::Type::VALUE && simulation->opacity.type != RE_PR_Opacity::Type::NONE && simulation->opacity.useCurve)
			{
				if(simulation->opacity.curve.DrawEditor("Opacity")) need_save = true;
			}
			else
				ImGui::Text("Select a opacity over type and enable curve at opacity propierties.");

			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Color Curve", NULL, wFlags))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (simulation->color.type != RE_PR_Color::Type::SINGLE && simulation->color.useCurve)
			{
				if(simulation->color.curve.DrawEditor("Color")) need_save = true;
			}
			else
				ImGui::Text("Select a color over type and enable curve at color propierties.");

			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		if (close) {
			if (need_save)
			{
				if (emiter_md5) {
					RE_ParticleEmitterBase* emitter = dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emiter_md5));
					bool has_emissor = emitter->HasEmissor(), has_render = emitter->HasRenderer();
					RE_EDITOR->popupWindow->PopUpSaveParticles((!has_emissor || !has_render), true, has_emissor, has_render, true);
				}
				else RE_EDITOR->popupWindow->PopUpSaveParticles(true, false, new_emitter->HasEmissor(), new_emitter->HasRenderer(), true);
			}
			else CloseEditor();
		}
	}
}
