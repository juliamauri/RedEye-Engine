#include "RE_Texture.h"

#include "Application.h"
#include "RE_FileSystem.h"
#include "RE_TextureImporter.h"
#include "RE_ThumbnailManager.h"
#include "RE_GLCache.h"

#include "OutputLog.h"

#include "Glew/include/glew.h"

#include "ImGui/imgui.h"

#define MINFCOMBO "Nearest\0Linear\0Nearest Mipmap Nearest\0Linear Mipmap Nearest\0Nearest Mipmap Linear\0Linear Mipmap Linear"
#define MAGFOMBO "Nearest\0Linear"
#define WRAPOMBO "Repeat\0Clamp to border\0Clamp to edge\0Mirrored Repeat"

RE_Texture::RE_Texture() { }

RE_Texture::RE_Texture(const char * metaPath) :ResourceContainer(metaPath) { }

RE_Texture::~RE_Texture()
{
}

const char* RE_Texture::GenerateMD5()
{
	const char* ret = nullptr;
	RE_FileIO generateMD5(GetAssetPath());
	eastl::string newMD5 = generateMD5.GetMd5();
	if (!newMD5.empty())
	{
		SetMD5(newMD5.c_str());
		ret = GetMD5();
		eastl::string libraryPath("Library/Textures/");
		libraryPath += ret;
		SetLibraryPath(libraryPath.c_str());
	}
	return ret;
}

TextureType RE_Texture::DetectExtension()
{
	eastl::string assetPath(GetAssetPath());
	eastl::string filename = assetPath.substr(assetPath.find_last_of("/") + 1);
	eastl::string extensionStr = filename.substr(filename.find_last_of(".") + 1);
	const char* extension = extensionStr.c_str();

	int size = eastl::CharStrlen(extension);

	if (size > 0) {
		if (eastl::Compare(extension, "dds", 3) == 0)
			texType = RE_DDS;
		else if (eastl::Compare(extension, "png", 3) == 0)
			texType = RE_PNG;
		else if (eastl::Compare(extension, "jpg", 3) == 0)
			texType = RE_JPG;
		else if (eastl::Compare(extension, "tga", 3) == 0)
			texType = RE_TGA;
		else if (eastl::Compare(extension, "tiff", 4) == 0)
			texType = RE_TIFF;
		else if (eastl::Compare(extension, "bmp", 3) == 0)
			texType = RE_BMP;
		else
			texType = RE_TEXTURE_UNKNOWN;
	}
	else
		texType = RE_TEXTURE_UNKNOWN;

	return texType;
}

TextureType RE_Texture::GetTextureType() const
{
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

void RE_Texture::Import(bool keepInMemory)
{
	AssetLoad();
	LibrarySave();
	if (!keepInMemory) UnloadMemory();
}

void RE_Texture::use()
{
	RE_GLCache::ChangeTextureBind(ID);
}

void RE_Texture::GetWithHeight(int * w, int * h)
{
	*w = width;
	*h = height;
}

void RE_Texture::DrawTextureImGui()
{
	ImGui::Image((void*)App->thumbnail->At(GetMD5()), ImVec2(256, 256), { 0.0, 1.0 }, {1.0, 0.0});
}

void RE_Texture::Draw()
{
	if (applySave) {
		if (ImGui::Button("Save")) {
			restoreSettings = texSettings;
			SaveMeta();
			applySave = false;
		}
		if (ImGui::Button("Restore")) {
			if (ResourceContainer::inMemory && ((texSettings.min_filter <= RE_LINEAR && restoreSettings.min_filter > RE_LINEAR)
				|| (restoreSettings.min_filter <= RE_LINEAR && texSettings.min_filter > RE_LINEAR)))
			{
				texSettings = restoreSettings;
				UnloadMemory();
				LoadInMemory();
			}
			else if (ResourceContainer::inMemory) {
				TexParameteri(GL_TEXTURE_MIN_FILTER, restoreSettings.min_filter);
				TexParameteri(GL_TEXTURE_MAG_FILTER, restoreSettings.mag_filter);
				TexParameteri(GL_TEXTURE_WRAP_S, restoreSettings.wrap_s);
				TexParameteri(GL_TEXTURE_WRAP_T, restoreSettings.wrap_t);
				if (restoreSettings.wrap_s == RE_TextureWrap::RE_CLAMP_TO_BORDER || restoreSettings.wrap_t == RE_TextureWrap::RE_CLAMP_TO_BORDER)
					TexParameterfv(GL_TEXTURE_BORDER_COLOR, &restoreSettings.borderColor[0]);
				texSettings = restoreSettings;
			}
			else
				texSettings = restoreSettings;

			applySave = false;
		}
	}

	int minIndex = GetComboFilter(texSettings.min_filter);
	if (ImGui::Combo("Minify filter", &minIndex, MINFCOMBO)) {
		RE_TextureFilters newfilter = GetFilterCombo(minIndex);
		if (texSettings.min_filter != newfilter) {
			if (ResourceContainer::inMemory && ((texSettings.min_filter <= RE_LINEAR && newfilter > RE_LINEAR)
				|| (newfilter <= RE_LINEAR && texSettings.min_filter > RE_LINEAR))) {
				texSettings.min_filter = newfilter;
				UnloadMemory();
				LoadInMemory();
			}
			else if (ResourceContainer::inMemory) {
				TexParameteri(GL_TEXTURE_MIN_FILTER, texSettings.min_filter = newfilter);
			}
			else
				texSettings.min_filter = newfilter;
			applySave = true;
		}
	}

	int magIndex = GetComboFilter(texSettings.mag_filter);
	if(ImGui::Combo("Magnify filter",&magIndex,MAGFOMBO)) {
		RE_TextureFilters newfilter = GetFilterCombo(magIndex);
		if (texSettings.mag_filter != newfilter) {
			if (ResourceContainer::inMemory) TexParameteri(GL_TEXTURE_MAG_FILTER, texSettings.mag_filter = newfilter);
			applySave = true;
		}
	}

	int wrapSIndex = GetComboWrap(texSettings.wrap_s);
	if(ImGui::Combo("Wrap S", &wrapSIndex ,WRAPOMBO)) {
		RE_TextureWrap newwrap = GetWrapCombo(wrapSIndex);
		if (texSettings.wrap_s != newwrap) {
			if (ResourceContainer::inMemory) TexParameteri(GL_TEXTURE_WRAP_S, texSettings.wrap_s = newwrap);
			applySave = true;
		}
	}

	int wrapTIndex = GetComboWrap(texSettings.wrap_t);
	if(ImGui::Combo("Wrap T", &wrapTIndex, WRAPOMBO)) {
		RE_TextureWrap newwrap = GetWrapCombo(wrapTIndex);
		if (texSettings.wrap_t != newwrap) {
			if (ResourceContainer::inMemory) TexParameteri(GL_TEXTURE_WRAP_T, texSettings.wrap_t = newwrap);
			applySave = true;
		}
	}
	
	math::float4 bColor = texSettings.borderColor;
	if (ImGui::ColorEdit4("Border Color", &bColor[0], ImGuiColorEditFlags_AlphaBar)) {
		if (!texSettings.borderColor.Equals(bColor))
		{
			if (ResourceContainer::inMemory && texSettings.wrap_s == RE_TextureWrap::RE_CLAMP_TO_BORDER || texSettings.wrap_t == RE_TextureWrap::RE_CLAMP_TO_BORDER)
				TexParameterfv(GL_TEXTURE_BORDER_COLOR, &texSettings.borderColor[0]);
			applySave = true;
		}
	}

	DrawTextureImGui();

	if (applySave && texSettings == restoreSettings) {
		applySave = false;
	}
}

void RE_Texture::SaveResourceMeta(JSONNode * metaNode)
{
	metaNode->PushInt("TextureType", texType);
	metaNode->PushInt("minFilter", texSettings.min_filter);
	metaNode->PushInt("magFilter", texSettings.mag_filter);
	metaNode->PushInt("wrapS", texSettings.wrap_s);
	metaNode->PushInt("wrapT", texSettings.wrap_t);
	metaNode->PushFloat4("borderColor", texSettings.borderColor);
}

void RE_Texture::LoadResourceMeta(JSONNode * metaNode)
{
	texType = (TextureType)metaNode->PullInt("TextureType", RE_TEXTURE_UNKNOWN);
	texSettings.min_filter = (RE_TextureFilters)metaNode->PullInt("minFilter", RE_NEAREST);
	texSettings.mag_filter = (RE_TextureFilters)metaNode->PullInt("magFilter", RE_NEAREST);
	texSettings.wrap_s = (RE_TextureWrap)metaNode->PullInt("wrapS", RE_REPEAT);
	texSettings.wrap_t = (RE_TextureWrap)metaNode->PullInt("wrapT", RE_REPEAT);
	texSettings.borderColor = metaNode->PullFloat4("borderColor", math::float4::zero);

	restoreSettings = texSettings;
}

int RE_Texture::GetComboFilter(RE_TextureFilters filter)
{
	int comboIndex = 0;
	switch (filter)
	{
	case RE_TextureFilters::RE_NEAREST:
		comboIndex = 0;
		break;
	case RE_TextureFilters::RE_LINEAR:
		comboIndex = 1;
		break;
	case RE_TextureFilters::RE_NEAREST_MIPMAP_NEAREST:
		comboIndex = 2;
		break;
	case RE_TextureFilters::RE_LINEAR_MIPMAP_NEAREST:
		comboIndex = 3;
		break;
	case RE_TextureFilters::RE_NEAREST_MIPMAP_LINEAR:
		comboIndex = 4;
		break;
	case RE_TextureFilters::RE_LINEAR_MIPMAP_LINEAR:
		comboIndex = 5;
		break;
	}
	return comboIndex;
}

RE_TextureFilters RE_Texture::GetFilterCombo(int combo)
{
	RE_TextureFilters ret;
	switch (combo)
	{
	case 0:
		ret = RE_TextureFilters::RE_NEAREST;
		break;
	case 1:
		ret = RE_TextureFilters::RE_LINEAR;
		break;
	case 2:
		ret = RE_TextureFilters::RE_NEAREST_MIPMAP_NEAREST;
		break;
	case 3:
		ret = RE_TextureFilters::RE_LINEAR_MIPMAP_NEAREST;
		break;
	case 4:
		ret = RE_TextureFilters::RE_NEAREST_MIPMAP_LINEAR;
		break;
	case 5:
		ret = RE_TextureFilters::RE_LINEAR_MIPMAP_LINEAR;
		break;
	}
	return ret;
}

int RE_Texture::GetComboWrap(RE_TextureWrap wrap)
{
	int comboIndex = 0;
	switch (wrap)
	{
	case RE_TextureWrap::RE_REPEAT:
		comboIndex = 0;
		break;
	case RE_TextureWrap::RE_CLAMP_TO_BORDER:
		comboIndex = 1;
		break;
	case RE_TextureWrap::RE_CLAMP_TO_EDGE:
		comboIndex = 2;
		break;
	case RE_TextureWrap::RE_MIRRORED_REPEAT:
		comboIndex = 3;
	}
	return comboIndex;
}

RE_TextureWrap RE_Texture::GetWrapCombo(int combo)
{
	RE_TextureWrap ret;
	switch (combo)
	{
	case 0:
		ret = RE_TextureWrap::RE_REPEAT;
		break;
	case 1:
		ret = RE_TextureWrap::RE_CLAMP_TO_BORDER;
		break;
	case 2:
		ret = RE_TextureWrap::RE_CLAMP_TO_EDGE;
		break;
	case 3:
		ret = RE_TextureWrap::RE_MIRRORED_REPEAT;
		break;
	}
	return ret;
}

void RE_Texture::ReImport()
{
	bool unload = false;
	if (isInMemory()) {
		unload = true;
		UnloadMemory();
	}
	AssetLoad();
	LibrarySave();
	if (!unload) UnloadMemory();
}

void RE_Texture::TexParameteri(unsigned int pname, int param)
{
	if (ResourceContainer::inMemory) {
		GLuint boundTexture = 0;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&boundTexture);
		glBindTexture(GL_TEXTURE_2D, ID);
		glTexParameteri(GL_TEXTURE_2D, pname, param);
		glBindTexture(GL_TEXTURE_2D, boundTexture);
	}
}

void RE_Texture::TexParameterfv(unsigned int pname, float * param)
{
	if (ResourceContainer::inMemory) {
		GLuint boundTexture = 0;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&boundTexture);
		glBindTexture(GL_TEXTURE_2D, ID);
		glTexParameterfv(GL_TEXTURE_2D, pname, param);
		glBindTexture(GL_TEXTURE_2D, boundTexture);
	}
}

void RE_Texture::AssetLoad()
{
	RE_FileIO assetFile(GetAssetPath());
	if (assetFile.Load()) {
		App->textures->LoadTextureInMemory(assetFile.GetBuffer(), assetFile.GetSize(), texType, &ID, &width, &height, texSettings);
		
		SetMD5(assetFile.GetMd5().c_str());
		eastl::string libraryPath("Library/Textures/");
		libraryPath += GetMD5();
		SetLibraryPath(libraryPath.c_str());

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