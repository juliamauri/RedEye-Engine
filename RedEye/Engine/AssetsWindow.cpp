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
#include <EASTL/bit.h>
#include <EASTL/array.h>

void AssetsWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar))
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
			DrawDirectories(currentDir, toChange);
			DrawDisplayOptions(iconsSize);
			ImGui::EndMenuBar();
		}

		float width = ImGui::GetWindowWidth();
		auto itemsColum = static_cast<int>(width / iconsSize);
		if (itemsColum == 0) itemsColum = 1;
		eastl::stack<RE_FileSystem::RE_Path*> filesToDisplay = currentDir->GetDisplayingFiles();

		if (ImGui::BeginTable("AssetsFiles", itemsColum, ImGuiTableFlags_::ImGuiTableFlags_NoBordersInBody)) {
			eastl::string idName = "#AssetImage";
			uint idCount = 0;

			while (!filesToDisplay.empty())
			{
				ImGui::TableNextColumn();
				DrawDirectoryItem(filesToDisplay, idName, idCount, iconsSize, toChange, secondary);
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

void AssetsWindow::DrawDirectoryItem(eastl::stack<RE_FileSystem::RE_Path*>& filesToDisplay, const eastl::string& idName, unsigned int& idCount, float iconsSize, RE_FileSystem::RE_Directory*& toChange, bool secondary)
{
	const RE_FileSystem::RE_Path* p = filesToDisplay.top();
	filesToDisplay.pop();
	idCount += 1;
	eastl::string id = idName + eastl::to_string(idCount);
	ImGui::PushID(id.c_str());
	switch (p->pType)
	{
	case RE_FileSystem::PathType::D_FOLDER:
	{
		DrawItemFolder(iconsSize, toChange, p);
		break;
	}
	case RE_FileSystem::PathType::D_FILE:
	{
		switch (p->AsFile()->fType)
		{
		case RE_FileSystem::FileType::F_META:
		{
			DrawItemMeta(p, iconsSize, id, idName, idCount);
			break;
		}
		case RE_FileSystem::FileType::F_NOTSUPPORTED:
		{
			DrawItemNotSupported(secondary, iconsSize, p, id, idName, idCount);
			break;
		}
		default:
		{
			DrawItemResource(p, iconsSize, id, idName, idCount);
			break;
		}
		}
		break;
	}
	default: ImGui::PopID(); break;
	}
}

void AssetsWindow::DrawItemResource(const RE_FileSystem::RE_Path* p, float iconsSize, eastl::string& id, const eastl::string& idName, const unsigned int& idCount) const
{
	if (p->AsFile()->metaResource != nullptr)
	{
		const ResourceContainer* res = RE_RES->At(p->AsFile()->metaResource->resource);

		uintptr_t file_icon = 0;

		switch (res->GetType())
		{
		case ResourceType::PARTICLE_EMISSION: file_icon = RE_EDITOR->thumbnails->GetPEmissionFileID(); break;
		case ResourceType::PARTICLE_RENDER: file_icon = RE_EDITOR->thumbnails->GetPRenderFileID(); break;
		default: file_icon = RE_EDITOR->thumbnails->At(res->GetMD5()); break;
		}
		
		if (ImGui::ImageButton(eastl::bit_cast<void*>(file_icon), { iconsSize, iconsSize }, { 0.0f, 1.0f }, { 1.0f, 0.0f }, 0))
			RE_RES->PushSelected(res->GetMD5(), true);
		ImGui::PopID();

		DrawPopUpDeleteResource(id, idName, idCount, res);
		
		static eastl::array<const char*, static_cast<unsigned short>(ResourceType::MAX)> names = { "Undefined", "Shader", "Texture", "Mesh", "Prefab", "SkyBox", "Material", "Model", "Scene", "Particle emitter", "Particle emission", "Particle render" };
		eastl::string dragID("#");
		(dragID += names[static_cast<unsigned short>(res->GetType())]) += "Reference";

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload(dragID.c_str(), &p->AsFile()->metaResource->resource, sizeof(const char**));
			ImGui::Image(eastl::bit_cast<void*>(file_icon), { 50,50 }, { 0.0f, 1.0f }, { 1.0f, 0.0f });
			ImGui::EndDragDropSource();
		}

		ImGui::Text(p->AsFile()->filename.c_str());
	}
	else
		ImGui::PopID();
}

void AssetsWindow::DrawItemNotSupported(bool secondary, float iconsSize, const RE_FileSystem::RE_Path* p, eastl::string& id, const eastl::string& idName, const unsigned int& idCount)
{
	bool pop = (!selectingUndefFile && !secondary);
	if (pop)
	{
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	}

	if (ImGui::ImageButton(
		eastl::bit_cast<void*>(selectingUndefFile ? RE_EDITOR->thumbnails->GetSelectFileID() : RE_EDITOR->thumbnails->GetFileID()),
		{ iconsSize, iconsSize }, { 0.0f, 1.0f }, { 1.0f, 0.0f }, selectingUndefFile ? -1 : 0)
		&& selectingUndefFile)
	{
		*selectingUndefFile = p->path;
		selectingUndefFile = nullptr;
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

	if ((pop = (!selectingUndefFile && !secondary)))
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
}

void AssetsWindow::DrawItemMeta(const RE_FileSystem::RE_Path* p, float iconsSize, eastl::string& id, const eastl::string& idName, const unsigned int& idCount) const
{
	const ResourceContainer* res = RE_RES->At(p->AsMeta()->resource);

	uintptr_t icon_meta = (res->GetType() == ResourceType::SHADER) ? RE_EDITOR->thumbnails->GetShaderFileID() : RE_EDITOR->thumbnails->GetPEmitterFileID();

	if (ImGui::ImageButton(eastl::bit_cast<void*>(icon_meta), { iconsSize, iconsSize }, { 0.0f, 1.0f }, { 1.0f, 0.0f }, 0))
		RE_RES->PushSelected(res->GetMD5(), true);

	if (ImGui::BeginDragDropSource())
	{
		ImGui::SetDragDropPayload((res->GetType() == ResourceType::SHADER) ? "#ShadereReference" : "#EmitterReference", &p->AsMeta()->resource, sizeof(const char**));
		ImGui::Image(eastl::bit_cast<void*>(icon_meta), { 50,50 }, { 0.0f, 1.0f }, { 1.0f, 0.0f });
		ImGui::EndDragDropSource();
	}
	ImGui::PopID();

	DrawPopUpDeleteResource(id, idName, idCount, res);

	ImGui::Text(res->GetName());
}

void AssetsWindow::DrawItemFolder(float iconsSize, RE_FileSystem::RE_Directory*& toChange, const RE_FileSystem::RE_Path* p) const
{
	if (ImGui::ImageButton(eastl::bit_cast<void*>(RE_EDITOR->thumbnails->GetFolderID()), { iconsSize, iconsSize }, { 0.0f, 1.0f }, { 1.0f, 0.0f }, 0))
		toChange = p->AsDirectory();
	ImGui::PopID();
	ImGui::Text(p->AsDirectory()->name.c_str());
}

void AssetsWindow::DrawPopUpDeleteResource(eastl::string& id, const eastl::string& idName, const unsigned int& idCount, const ResourceContainer* res) const
{
	id = idName + eastl::to_string(idCount) + "Delete";
	ImGui::PushID(id.c_str());
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::Button("Delete")) RE_EDITOR->popupWindow->PopUpDelRes(res->GetMD5());
		ImGui::EndPopup();
	}
	ImGui::PopID();
}

void AssetsWindow::DrawDisplayOptions(float& iconsSize)
{
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100);
	ImGui::SliderFloat("Icons size", &iconsSize, 25, 256, "%.0f");
	if (selectingUndefFile)
	{
		ImGui::SameLine();
		if (ImGui::Button("Cancel selection")) selectingUndefFile = nullptr;
	}
}

void AssetsWindow::DrawDirectories(RE_FileSystem::RE_Directory* currentDir, RE_FileSystem::RE_Directory*& toChange)
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
}
