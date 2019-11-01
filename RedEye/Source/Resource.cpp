#include "Resource.h"

#include "Application.h"
#include "FileSystem.h"

#include "Globals.h"
#include "RE_Mesh.h"

#include "ImGui/imgui.h"

ResourceContainer::ResourceContainer()
{
}

ResourceContainer::ResourceContainer(const char * _metaPath)
{
	metaPath = _metaPath;
	LoadMeta();
}

ResourceContainer::~ResourceContainer()
{
	if (md5)
		DEL_A(md5);
}

const char * ResourceContainer::GetName() const
{
	return name.c_str();
}

const char * ResourceContainer::GetAssetPath() const
{
	return assetPath.c_str();
}

const char * ResourceContainer::GetMetaPath() const
{
	return metaPath.c_str();
}

const char* ResourceContainer::GetMD5() const
{
	return md5;
}

Resource_Type ResourceContainer::GetType() const
{
	return type;
}

void ResourceContainer::SetType(Resource_Type type)
{
	this->type = type;

	switch (type)
	{
	case R_UNDEFINED:
		propietiesName = "Undefined";
		break;
	case R_SHADER:
		propietiesName = "Shader";
		break;
	case R_PRIMITIVE:
		propietiesName = "Primitive";
		break;
	case R_TEXTURE:
		propietiesName = "Texture";
		break;
	case R_MESH:
		propietiesName = "Mesh";
		break;
	case R_PREFAB:
		propietiesName = "Prefab";
		break;
	case R_SKYBOX:
		propietiesName = "SkyBox";
		break;
	case R_INTERNALPREFAB:
		propietiesName = "Internal preafab";
		break;
	case R_MATERIAL:
		propietiesName = "Material";
		break;
	default:
		propietiesName = "";
		break;
	}
	propietiesName += "resource";
}

void ResourceContainer::SetMD5(const char * _md5)
{
	if (md5)
	{
		DEL_A(md5);
		md5 = nullptr;
	}
	if (_md5)
	{
		std::string str(_md5);
		char* writtable = new char[str.size() + 1];
		std::copy(str.begin(), str.end(), writtable);
		writtable[str.size()] = '\0';
		md5 = writtable;
	}
}

void ResourceContainer::SetLibraryPath(const char * path)
{
	libraryPath = path;
}

void ResourceContainer::SetAssetPath(const char * originPath)
{
	assetPath = originPath;
	metaPath = assetPath.substr(0, assetPath.find_last_of(".") - 1);
	metaPath = ".meta";
}

void ResourceContainer::SetName(const char * _name)
{
	name = _name;
}

void ResourceContainer::SaveMeta()
{
	Config metaSerialize(metaPath.c_str(), App->fs->GetZipPath());

	JSONNode* metaNode = metaSerialize.GetRootNode("meta");

	metaNode->PushString("Name", name.c_str());
	metaNode->PushString("AssetPath", assetPath.c_str);
	metaNode->PushString("LibraryPath", libraryPath.c_str);
	metaNode->PushString("MD5", md5);
	metaNode->PushInt("Type", type);

	SaveResourceMeta(metaNode);
	metaSerialize.Save();
	DEL(metaNode);
}

void ResourceContainer::LoadMeta()
{
	Config metaDeserialize(metaPath.c_str(), App->fs->GetZipPath());
	if (metaDeserialize.Load()) {
		JSONNode* metaNode = metaDeserialize.GetRootNode("meta");

		name = metaNode->PullString("Name", "unkown");
		libraryPath = metaNode->PullString("AssetPath", "Assets/");
		assetPath = metaNode->PullString("LibraryPath", "Library/");
		md5 = metaNode->PullString("MD5", "no MD5");
		type = (Resource_Type)metaNode->PullInt("Type", Resource_Type::R_UNDEFINED);

		LoadResourceMeta(metaNode);

		DEL(metaNode);
	}
}

void ResourceContainer::DrawPropieties()
{
	if (ImGui::CollapsingHeader(propietiesName.c_str())) {

		ImGui::Text("Name: %s", name.c_str());
		ImGui::Text("Asset path: %s", assetPath.c_str());
		ImGui::Text("Library path: %s", libraryPath.c_str());
		ImGui::Text("Meta path: %s", metaPath.c_str());
		ImGui::Text("MD5: %s", md5);

		ImGui::Separator();

		Draw();
	}
}

const char * ResourceContainer::GetLibraryPath() const
{
	return libraryPath.c_str();
}
