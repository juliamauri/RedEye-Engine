#include "RE_Texture.h"

#include "Application.h"
#include "FileSystem.h"
#include "RE_TextureImporter.h"

#include "OutputLog.h"

#include "Glew/include/glew.h"

#include "ImGui/imgui.h"

RE_Texture::RE_Texture() { }

RE_Texture::RE_Texture(const char * metaPath) :ResourceContainer(metaPath) { }

RE_Texture::~RE_Texture()
{
}

const char* RE_Texture::GenerateMD5()
{
	const char* ret = nullptr;
	RE_FileIO generateMD5(GetAssetPath());
	std::string newMD5(generateMD5.GetMd5());
	if (!newMD5.empty())
	{
		SetMD5(newMD5.c_str());
		ret = GetMD5();
		std::string libraryPath("Library/Textures/");
		libraryPath += ret;
		SetLibraryPath(libraryPath.c_str());
	}
	return ret;
}

TextureType RE_Texture::DetectExtension()
{
	std::string assetPath(GetAssetPath());
	std::string filename = assetPath.substr(assetPath.find_last_of("/") + 1);
	std::string extensionStr = filename.substr(filename.find_last_of(".") + 1);
	const char* extension = extensionStr.c_str();

	if (std::strcmp(extension, "dds") == 0)
		texType = RE_DDS;
	else if (std::strcmp(extension, "png") == 0)
		texType = RE_PNG;
	else if (std::strcmp(extension, "jpg") == 0)
		texType = RE_JPG;
	else if (std::strcmp(extension, "tga") == 0)
		texType = RE_TGA;
	else if (std::strcmp(extension, "tiff") == 0)
		texType = RE_TIFF;
	else if (std::strcmp(extension, "bmp") == 0)
		texType = RE_BMP;
	else
		texType = RE_TEXTURE_UNKNOWN;

	return texType;
}

void RE_Texture::LoadInMemory()
{
	if (App->fs->Exists(GetLibraryPath()))
		LibraryLoad();
	else if (App->fs->Exists(GetAssetPath())) {
		AssetLoad();
		LibrarySave();
	}
	else {
		LOG_ERROR("Texture %s not found on project", GetName());
	}
}

void RE_Texture::UnloadMemory()
{
	glDeleteTextures(1, &ID);
	ID = 0;
	ResourceContainer::inMemory = false;
}

void RE_Texture::use()
{
	glBindTexture(GL_TEXTURE_2D, ID);
}

void RE_Texture::GetWithHeight(int * w, int * h)
{
	*w = width;
	*h = height;
}

void RE_Texture::DrawTextureImGui()
{
	ImGui::Image((void *)ID, ImVec2(200, 200));
}

void RE_Texture::AssetLoad()
{
	RE_FileIO assetFile(GetAssetPath());
	if (assetFile.Load()) {
		App->textures->LoadTextureInMemory(assetFile.GetBuffer(), assetFile.GetSize(), texType, &ID, &width, &height, texSettings);
		ResourceContainer::inMemory = true;
	}
}

void RE_Texture::LibraryLoad()
{
	RE_FileIO libraryFile(GetLibraryPath());
	if (libraryFile.Load()) {
		App->textures->LoadTextureInMemory(libraryFile.GetBuffer(), libraryFile.GetSize(), TextureType::RE_DDS, &ID, &width, &height, texSettings);
		ResourceContainer::inMemory = true;
	}
}

void RE_Texture::LibrarySave()
{
	RE_FileIO assetFile(GetAssetPath());
	RE_FileIO libraryFile(GetLibraryPath(), App->fs->GetZipPath());
	if (assetFile.Load()) {
		App->textures->SaveOwnFormat(assetFile.GetBuffer(), assetFile.GetSize(), texType, &libraryFile);
	}
}
