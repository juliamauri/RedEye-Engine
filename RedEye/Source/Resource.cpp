#include "Resource.h"

#include "Application.h"
#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"

#include "Globals.h"

#include "ImGui/imgui.h"

#define MD5SIZE 32

ResourceContainer::ResourceContainer()
{
	(md5 = new char[MD5SIZE + 1])[MD5SIZE] = '\0';
}

ResourceContainer::ResourceContainer(const char * _metaPath)
{
	(md5 = new char[MD5SIZE + 1])[MD5SIZE] = '\0';
	metaPath = _metaPath;
}

ResourceContainer::~ResourceContainer()
{
	if (isInMemory()) UnloadMemory();
	if (md5) DEL_A(md5);
}

const char * ResourceContainer::GetName() const { return name.c_str(); }
const char * ResourceContainer::GetAssetPath() const { return assetPath.c_str(); }
const char * ResourceContainer::GetMetaPath() const { return metaPath.c_str(); }
const char* ResourceContainer::GetMD5() const { return md5; }
Resource_Type ResourceContainer::GetType() const { return type; }
signed long long ResourceContainer::GetLastTimeModified() const { return lastModified; }

void ResourceContainer::SetType(Resource_Type _type)
{
	static const char* names[MAX_R_TYPES] = { "Undefined ", "Shader ", "Texture ", "Mesh ", "Prefab ", "SkyBox ", "Material ", "Model ", "Scene " };
	(propietiesName = names[type = _type]) += "resource ";
}

void ResourceContainer::SetMD5(const char * _md5)
{
	if (_md5) memcpy(md5, _md5, sizeof(char) * MD5SIZE);
}

void ResourceContainer::SetLibraryPath(const char * path) { libraryPath = path; }

void ResourceContainer::SetAssetPath(const char * originPath)
{
	metaPath = assetPath = originPath;
	metaPath += ".meta";
}

void ResourceContainer::SetMetaPath(const char* originPath)
{
	metaPath = originPath;
	metaPath += name + ".meta";
}

void ResourceContainer::SetName(const char * _name) { name = _name; }
void ResourceContainer::SetInternal(bool is_internal) { isinternal = is_internal; assetPath = "Internal Resources"; }

void ResourceContainer::SaveMeta()
{
	Config metaSerialize(metaPath.c_str(), App::fs->GetZipPath());

	JSONNode* metaNode = metaSerialize.GetRootNode("meta");

	metaNode->PushString("Name", name.c_str());
	metaNode->PushString("AssetPath", assetPath.c_str());
	metaNode->PushString("LibraryPath", libraryPath.c_str());
	metaNode->PushString("MD5", md5);
	metaNode->PushInt("Type", type);
	metaNode->PushSignedLongLong("lastModified", lastModified);

	SaveResourceMeta(metaNode);
	metaSerialize.Save();
	DEL(metaNode);
}

void ResourceContainer::LoadMeta()
{
	Config metaDeserialize(metaPath.c_str(), App::fs->GetZipPath());
	if (metaDeserialize.Load()) {
		JSONNode* metaNode = metaDeserialize.GetRootNode("meta");

		name = metaNode->PullString("Name", "unkown");
		assetPath = metaNode->PullString("AssetPath", "Assets/");
		libraryPath = metaNode->PullString("LibraryPath", "Library/");
		SetMD5(metaNode->PullString("MD5", "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"));
		SetType((Resource_Type)metaNode->PullInt("Type", Resource_Type::R_UNDEFINED));
		lastModified  = metaNode->PullSignedLongLong("lastModified", 0);

		LoadResourceMeta(metaNode);
		DEL(metaNode);
	}
}

void ResourceContainer::DrawPropieties()
{
	if (ImGui::CollapsingHeader(eastl::string(propietiesName + name).c_str(), ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen)) {
		if (ImGui::Button("Return")) App::resources->PopSelected();
		ImGui::Separator();
		ImGui::Text("Name: %s", name.c_str());
		ImGui::Text("Asset path: %s", assetPath.c_str());
		ImGui::Text("Library path: %s", libraryPath.c_str());
		ImGui::Text("Meta path: %s", metaPath.c_str());
		ImGui::Text("MD5: %s", md5);
		ImGui::Text("Times Counted: %u", App::resources->TotalReferenceCount(md5));

		ImGui::Separator();
		Draw();
	}
}

const char * ResourceContainer::GetLibraryPath() const { return libraryPath.c_str(); }