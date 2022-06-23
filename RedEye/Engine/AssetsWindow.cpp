#include "EditorWindow.h"
#include <EASTL/string.h>

#include "AssetsWindow.h"

#include "Application.h"
#include "ModuleEditor.h"
#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"
#include "RE_ThumbnailManager.h"
#include "PopUpWindow.h"

#include <ImGui/imgui_internal.h>

void AssetsWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		static RE_FileSystem::RE_Directory* currentDir = RE_FS->GetRootDirectory();
		RE_FileSystem::RE_Directory* toChange = nullptr;
		static float iconsSize = 100;

		if (ImGui::BeginMenuBar())
		{
			if (currentDir->parent == nullptr)
			{
				ImGui::Text("%s Folder", currentDir->name.c_str());
				currentPath = currentDir->path.c_str();
			}
			else
			{
				eastl::list<RE_FileSystem::RE_Directory*> folders = currentDir->FromParentToThis();
				for (auto dir : folders)
				{
					if (dir == currentDir) ImGui::Text(currentDir->name.c_str());
					else if (ImGui::Button(dir->name.c_str())) toChange = dir;

					if (dir != *folders.rbegin()) ImGui::SameLine();
				}
			}

			ImGui::SameLine();
			ImGui::SetNextItemWidth(100);
			ImGui::SliderFloat("Icons size", &iconsSize, 25, 256, "%.0f");
			if (selectingUndefFile)
			{
				ImGui::SameLine();
				if (ImGui::Button("Cancel selection")) selectingUndefFile = nullptr;
			}

			ImGui::EndMenuBar();
		}

		float width = ImGui::GetWindowWidth();
		int itemsColum = static_cast<int>(width / iconsSize);
		if (itemsColum == 0) itemsColum = 1;
		eastl::stack<RE_FileSystem::RE_Path*> filesToDisplay = currentDir->GetDisplayingFiles();

		if (ImGui::BeginTable("AssetsFiles", itemsColum, ImGuiTableFlags_::ImGuiTableFlags_NoBordersInBody)) {
			eastl::string idName = "#AssetImage";
			uint idCount = 0;


			while (!filesToDisplay.empty())
			{

				ImGui::TableNextColumn();

				RE_FileSystem::RE_Path* p = filesToDisplay.top();
				filesToDisplay.pop();
				eastl::string id = idName + eastl::to_string(idCount++);
				ImGui::PushID(id.c_str());
				switch (p->pType)
				{
				case RE_FileSystem::PathType::D_FOLDER:
				{
					if (ImGui::ImageButton(reinterpret_cast<void*>(RE_EDITOR->thumbnails->GetFolderID()), { iconsSize, iconsSize }, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0))
						toChange = p->AsDirectory();
					ImGui::PopID();
					ImGui::Text(p->AsDirectory()->name.c_str());
					break;
				}
				case RE_FileSystem::PathType::D_FILE:
				{
					switch (p->AsFile()->fType)
					{
					case RE_FileSystem::FileType::F_META:
					{
						ResourceContainer* res = RE_RES->At(p->AsMeta()->resource);

						unsigned int icon_meta = (res->GetType() == R_SHADER) ? RE_EDITOR->thumbnails->GetShaderFileID() : RE_EDITOR->thumbnails->GetPEmitterFileID();

						if (ImGui::ImageButton(reinterpret_cast<void*>(icon_meta), { iconsSize, iconsSize }, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0))
							RE_RES->PushSelected(res->GetMD5(), true);

						if (ImGui::BeginDragDropSource())
						{
							ImGui::SetDragDropPayload((res->GetType() == R_SHADER) ? "#ShadereReference" : "#EmitterReference", &p->AsMeta()->resource, sizeof(const char**));
							ImGui::Image(reinterpret_cast<void*>(icon_meta), { 50,50 }, { 0.0f, 0.0f }, { 1.0f, 1.0f });
							ImGui::EndDragDropSource();
						}
						ImGui::PopID();

						id = idName + eastl::to_string(idCount) + "Delete";
						ImGui::PushID(id.c_str());
						if (ImGui::BeginPopupContextItem())
						{
							if (ImGui::Button("Delete")) RE_EDITOR->popupWindow->PopUpDelRes(res->GetMD5());
							ImGui::EndPopup();
						}
						ImGui::PopID();

						ImGui::Text(res->GetName());
						break;
					}
					case RE_FileSystem::FileType::F_NOTSUPPORTED:
					{
						bool pop = (!selectingUndefFile && !secondary);
						if (pop)
						{
							ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
							ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
						}

						if (ImGui::ImageButton(
							reinterpret_cast<void*>(selectingUndefFile ? RE_EDITOR->thumbnails->GetSelectFileID() : RE_EDITOR->thumbnails->GetFileID()),
							{ iconsSize, iconsSize }, { 0.0f, 0.0f }, { 1.0f, 1.0f }, (selectingUndefFile) ? -1 : 0))
						{
							if (selectingUndefFile)
							{
								*selectingUndefFile = p->path;
								selectingUndefFile = nullptr;
							}
						}
						ImGui::PopID();

						if (pop)
						{
							ImGui::PopItemFlag();
							ImGui::PopStyleVar();
						}

						id = idName + eastl::to_string(idCount) + "Delete";
						ImGui::PushID(id.c_str());
						if (ImGui::BeginPopupContextItem())
						{
							if (ImGui::Button("Delete")) RE_EDITOR->popupWindow->PopUpDelUndeFile(p->path.c_str());
							ImGui::EndPopup();
						}
						ImGui::PopID();

						if (pop = (!selectingUndefFile && !secondary))
						{
							ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
							ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
						}

						ImGui::Text(p->AsFile()->filename.c_str());

						if (pop)
						{
							ImGui::PopItemFlag();
							ImGui::PopStyleVar();
						}

						break;
					}
					default:
					{
						if (p->AsFile()->metaResource != nullptr)
						{
							ResourceContainer* res = RE_RES->At(p->AsFile()->metaResource->resource);

							unsigned int file_icon = 0;

							switch (res->GetType())
							{
							case R_PARTICLE_EMISSION: file_icon = RE_EDITOR->thumbnails->GetPEmissionFileID(); break;
							case R_PARTICLE_RENDER: file_icon = RE_EDITOR->thumbnails->GetPRenderFileID(); break;
							default: file_icon = RE_EDITOR->thumbnails->At(res->GetMD5()); break;
							}

							if (ImGui::ImageButton(reinterpret_cast<void*>(file_icon), { iconsSize, iconsSize }, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0))
								RE_RES->PushSelected(res->GetMD5(), true);
							ImGui::PopID();

							id = idName + eastl::to_string(idCount) + "Delete";
							ImGui::PushID(id.c_str());
							if (ImGui::BeginPopupContextItem())
							{
								if (ImGui::Button("Delete")) RE_EDITOR->popupWindow->PopUpDelRes(res->GetMD5());
								ImGui::EndPopup();
							}
							ImGui::PopID();

							static const char* names[MAX_R_TYPES] = { "Undefined", "Shader", "Texture", "Mesh", "Prefab", "SkyBox", "Material", "Model", "Scene", "Particle emitter", "Particle emission", "Particle render" };
							eastl::string dragID("#");
							(dragID += names[res->GetType()]) += "Reference";

							if (ImGui::BeginDragDropSource())
							{
								ImGui::SetDragDropPayload(dragID.c_str(), &p->AsFile()->metaResource->resource, sizeof(const char**));
								ImGui::Image(reinterpret_cast<void*>(file_icon), { 50,50 }, { 0.0f, 0.0f }, { 1.0f, 1.0f });
								ImGui::EndDragDropSource();
							}

							ImGui::Text(p->AsFile()->filename.c_str());
						}
						else
							ImGui::PopID();

						break;
					}
					}
					break;
				}
				}
			}
			ImGui::EndTable();
		}

		if (secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		if (toChange)
		{
			currentDir = toChange;
			currentPath = currentDir->path.c_str();
		}
	}

	ImGui::End();
}