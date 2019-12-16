#include "RE_ThumbnailManager.h"

#include "Application.h"
#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"
#include "RE_TextureImporter.h"

#include "RE_Texture.h"

#include "Glew/include/glew.h"

#include <string>

#define THUMBNAILPATH "Library/Thumbnails/"

#define DEFTHUMBNAILS "Settings/Icons/"

RE_ThumbnailManager::RE_ThumbnailManager()
{
}

RE_ThumbnailManager::~RE_ThumbnailManager()
{
	glDeleteTextures(1, &folder);
	glDeleteTextures(1, &file);
	glDeleteTextures(1, &selectfile);
	glDeleteTextures(1, &shaderFile);
}

void RE_ThumbnailManager::Init()
{
	folder = LoadDefIcon("folder.dds");
	file = LoadDefIcon("file.dds");
	selectfile = LoadDefIcon("selectfile.dds");
	shaderFile = LoadDefIcon("shaderFile.dds");
}

void RE_ThumbnailManager::Add(const char* ref)
{
	ResourceContainer* res = App->resources->At(ref);

	switch (res->GetType())
	{
	case Resource_Type::R_TEXTURE:
		break;
	case Resource_Type::R_MATERIAL:
		break;
	case Resource_Type::R_MODEL:
	case Resource_Type::R_PREFAB:
	case Resource_Type::R_SCENE:
		break;
	case Resource_Type::R_SHADER:
		break;
	case Resource_Type::R_SKYBOX:
		break;
	}

	std::string tPath = THUMBNAILPATH;
	tPath += ref;
}

unsigned int RE_ThumbnailManager::At(const char* ref)const
{
	return thumbnails.at(ref);
}

unsigned int RE_ThumbnailManager::LoadDefIcon(const char* filename)
{
	unsigned int ret = 0;
	RE_TextureSettings defTexSettings;
	std::string path(DEFTHUMBNAILS);
	path += filename;
	RE_FileIO filderIcon(path.c_str());
	if (filderIcon.Load()) {
		int tmp1, tmp2;
		App->textures->LoadTextureInMemory(filderIcon.GetBuffer(), filderIcon.GetSize(), TextureType::RE_DDS, &ret, &tmp1, &tmp2, defTexSettings);
	}
	return ret;
}
