#include "RE_Prefab.h"
#include "Globals.h"

#include "Application.h"
#include "RE_GameObject.h"
#include "FileSystem.h"

#include "md5.h"
#include "ResourceManager.h"


RE_Prefab::RE_Prefab(RE_GameObject* toBePrefab, bool isInternal) : ResourceContainer(toBePrefab->GetName()), isInternal(isInternal)
{
	SetType((isInternal) ? Resource_Type::R_INTERNALPREFAB : Resource_Type::R_PREFAB);

	std::string save_path((isInternal) ? "Library/Scenes/" : "Assets/Prefabs/");

	save_path += GetName();
	//Internal is named .efab
	//external is named .refab
	save_path += (isInternal) ? ".efab" : ".refab";

	SetLibraryPath(save_path.c_str());

	Config prefab_file(save_path.c_str(), App->fs->GetZipPath());

	JSONNode* node = prefab_file.GetRootNode("Game Objects");

	node->SetArray();
	toBePrefab->Serialize(node);

	DEL(node);

	SetMD5(prefab_file.GetMd5().c_str());

	prefab_file.Save();
}

RE_Prefab::~RE_Prefab()
{
	if (loaded) DEL(loaded);
}

RE_GameObject * RE_Prefab::GetRoot()
{
	if (!loaded) Load();
	return new RE_GameObject(*loaded);
}

void RE_Prefab::Load()
{
	Config prefab_file(GetLibraryPath(), App->fs->GetZipPath());
	if (prefab_file.Load())
	{
		JSONNode* node = prefab_file.GetRootNode("Game Objects");
		loaded = node->FillGO();
		DEL(node);
	}
}

void RE_Prefab::Unload()
{
	DEL(loaded);
}
