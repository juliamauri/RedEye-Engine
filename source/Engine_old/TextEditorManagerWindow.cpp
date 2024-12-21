#include "EditorWindow.h"
#include <EASTL/string.h>
#include <EASTL/vector.h>

#include "TextEditorManagerWindow.h"

#include "RE_Memory.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "RE_FileSystem.h"
#include "RE_FileBuffer.h"
#include "RE_ShaderImporter.h"

#include <ImGui/imgui_internal.h>
#include <ImGuiImpl/imgui_stdlib.h>
#include <ImGuiWidgets/ImGuiColorTextEdit/TextEditor.h>

TextEditorManagerWindow::~TextEditorManagerWindow()
{
	for (auto e : editors)
	{
		if (e->file) DEL(e->file)
		if (e->textEditor) DEL(e->textEditor)
		DEL(e)
	}
}

void TextEditorManagerWindow::PushEditor(const char* filePath, eastl::string* newFile, const char* shadertTemplate, bool* open)
{
	for (auto e : editors)
		if (strcmp(e->toModify->c_str(), filePath) == 0)
			return;

	EditorData* e = new EditorData();
	if (filePath)
	{
		RE_FileBuffer* file = new RE_FileBuffer(filePath);
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
		if (filePath) DEL(e->file)
		DEL(e)
	}

	if (e)e->open = open;
}

void TextEditorManagerWindow::Draw(bool secondary)
{
	static const char* compileAsStr[4] = { "None", "Vertex", "Fragment", "Geometry" };
	static eastl::vector<EditorData*> toRemoveE;
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
				RE_LOGGER::ScopeProcedureLogging();

				std::string tmp = e->textEditor->GetText();
				eastl::string text(tmp.c_str(), tmp.size());
				e->works = RE_ShaderImporter::Compile(text.c_str(), text.size());
				if (!e->works) RE_LOG_ERROR("%s", RE_ShaderImporter::GetShaderError());
				e->compiled = true;

				RE_LOGGER::EndScope();
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
						e->file = new RE_FileBuffer(assetPath.c_str());
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
			DEL(e->textEditor)
			if (e->file) DEL(e->file)
			toRemoveE.push_back(e);
		}
	}

	if (!toRemoveE.empty())
	{
		//https://stackoverflow.com/questions/21195217/elegant-way-to-remove-all-elements-of-a-vector-that-are-contained-in-another-vec
		editors.erase(eastl::remove_if(eastl::begin(editors), eastl::end(editors),
			[&](auto x) {return eastl::find(begin(toRemoveE), end(toRemoveE), x) != end(toRemoveE); }), eastl::end(editors));
		for (auto e : toRemoveE) DEL(e)
		toRemoveE.clear();
	}
}