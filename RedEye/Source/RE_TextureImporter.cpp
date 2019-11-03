#include "RE_TextureImporter.h"

#include "Application.h"
#include "ResourceManager.h"
#include "FileSystem.h"

#include "RE_Texture.h"

#include "ModuleEditor.h"
#include "EditorWindows.h"

#include "Globals.h"
#include "OutputLog.h"
#include "TimeManager.h"

#include "md5.h"

#include "ImGui\imgui.h"

#include "Glew/include/glew.h"

#include "IL/include/il.h"
#include "IL/include/ilu.h"
#include "IL/include/ilut.h"

#pragma comment(lib, "IL/libx86/DevIL.lib")
#pragma comment(lib, "IL/libx86/ILU.lib")
#pragma comment(lib, "IL/libx86/ILUT.lib")



RE_TextureImporter::RE_TextureImporter(const char* folderPath) : folderPath(folderPath) {}

RE_TextureImporter::~RE_TextureImporter() {}

bool RE_TextureImporter::Init()
{
	LOG("Initializing Texture Manager");

	ilInit();
	iluInit();
	ilutInit();

	ILenum error = ilGetError();
	bool ret = (error == IL_NO_ERROR);
	if (ret)
		ilutRenderer(ILUT_OPENGL);
	else
		LOG_ERROR("DevIL could not initialice! DevIL Error %d - %s", error, iluErrorString(error));

	if (folderPath == nullptr)
	{
		LOG_ERROR("Texure Manager could not read folder path");
		ret = false;
	}

	char tmp[8];
	sprintf_s(tmp, 8, "%u.%u.%u", IL_VERSION / 100, (IL_VERSION % 100) / 10, IL_VERSION % 10);
	App->ReportSoftware("DevIL", tmp, "http://openil.sourceforge.net/");

	return ret;
}

const char * RE_TextureImporter::AddNewTextureOnResources(const char * assetsPath)
{
	const char* retMD5 = nullptr;
	std::string path(assetsPath);
	std::string filename = path.substr(path.find_last_of("/") + 1);
	std::string name = filename.substr(0, filename.find_last_of(".") - 1);
	std::string extension = filename.substr(filename.find_last_of(".") + 1);

	RE_Texture* newTexture = new RE_Texture();
	newTexture->SetAssetPath(path.c_str());
	if (newTexture->DetectExtension() != RE_TEXTURE_UNKNOWN) {
		newTexture->SetName(name.c_str());
		newTexture->SetType(Resource_Type::R_TEXTURE);
		retMD5 = newTexture->GenerateMD5();
		newTexture->SaveMeta();
	}
	else
	{
		LOG_ERROR("Error detecting texture extension. Don't suported %s", extension.c_str());
	}
	return retMD5;
}

unsigned int RE_TextureImporter::LoadSkyBoxTextures(const char * texturesPath, const char* extension)
{
	std::vector<std::string> skyBoxTexturesPath = App->fs->FindAllFilesByExtension(texturesPath, extension);
	App->editor->skyBoxWindow->SetSkyBoxPath(texturesPath);
	
	uint ID = 0;
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

	std::string skyBoxTextures[6];
	uint count = 0;
	for (auto texturePath : skyBoxTexturesPath) {
		RE_FileIO texData(texturePath.c_str());

		if (texData.Load()) {
			skyBoxTextures[count] = texturePath.c_str();

			uint imageID = 0;

			ilGenImages(1, &imageID);
			ilBindImage(imageID);

			if (IL_FALSE != ilLoadL(GetExtensionIL(extension), texData.GetBuffer(), texData.GetSize())) {

				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + count++,
					0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData()
				);

				ilBindImage(0);
				ilDeleteImages(1, &imageID);
			}
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	App->editor->skyBoxWindow->SetTextures(skyBoxTextures);

	return ID;
}

unsigned int RE_TextureImporter::LoadSkyBoxTextures(const char * texturesPath[6])
{
	uint ID = 0;
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

	for (uint i = 0; i < 6; i++) {
		std::string path(texturesPath[i]);
		std::string filename = path.substr(path.find_last_of("/") + 1);
		std::string extension = filename.substr(filename.find_last_of(".") + 1);

		RE_FileIO texData(texturesPath[i]);

		if (texData.Load()) {

			uint imageID = 0;

			ilGenImages(1, &imageID);
			ilBindImage(imageID);
			
			int ILextension = GetExtensionIL(extension.c_str());

			if (IL_FALSE != ilLoadL(ILextension, texData.GetBuffer(), texData.GetSize())) {

				if (ILextension == IL_TGA)
					iluFlipImage();

				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData()
				);

				ilBindImage(0);
				ilDeleteImages(1, &imageID);
			}
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return ID;
}

void RE_TextureImporter::LoadTextureInMemory(const char * buffer, unsigned int size, TextureType type, unsigned int * ID, int * width, int * height, RE_TextureSettings settings)
{
	uint imageID = 0;
	ilGenImages(1, &imageID);
	ilBindImage(imageID);

	if (IL_FALSE != ilLoadL(type, buffer, size)) {

		if (type == IL_TGA)
			iluFlipImage();

		/* OpenGL texture binding of the image loaded by DevIL  */
		glGenTextures(1, ID); /* Texture name generation */
		glBindTexture(GL_TEXTURE_2D, *ID); /* Binding of texture name */
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, settings.mag_filter); /* We will use linear interpolation for magnification filter */
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, settings.min_filter); /* We will use linear interpolation for minifying filter */
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, settings.wrap_s); /* We will use linear interpolation for minifying filter */
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, settings.wrap_t); /* We will use linear interpolation for minifying filter */

		if(settings.wrap_s == RE_TextureWrap::GL_CLAMP_TO_BORDER || settings.wrap_t == RE_TextureWrap::GL_CLAMP_TO_BORDER)
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &settings.borderColor[0]);

		glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), *width = ilGetInteger(IL_IMAGE_WIDTH), *height = ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData()); /* Texture specification */
		if(settings.min_filter >= RE_TextureFilters::RE_NEAREST_MIPMAP_NEAREST)
			glGenerateMipmap(GL_TEXTURE_2D);

		ilBindImage(0);
		/* Delete used resources*/
		ilDeleteImages(1, &imageID); /* Because we have already copied image data into texture data we can release memory used by image. */
	}
	else {
		LOG_ERROR("Error when loading texture on DevIL");
	}
}

void RE_TextureImporter::SaveOwnFormat(const char * assetBuffer, unsigned int assetSize, TextureType assetType, RE_FileIO * toSave)
{
	uint imageID = 0;
	ilGenImages(1, &imageID);
	ilBindImage(imageID);

	if (IL_FALSE != ilLoadL(assetType, assetBuffer, assetSize)) {
		if (assetType == IL_TGA)
			ilFlipSurfaceDxtcData();

		//Save into dds
		ILuint   size = ilSaveL(IL_DDS, NULL, 0); // Get the size of the data buffer
		ILubyte *data = new ILubyte[size];

		ilSaveL(IL_DDS, data, size); // Save with the ilSaveIL function

		toSave->Save((char*)data, size);

		ilBindImage(0);
		/* Delete used resources*/
		ilDeleteImages(1, &imageID); /* Because we have already copied image data into texture data we can release memory used by image. */
	} {
		LOG_ERROR("Error when loading texture on DevIL");
	}
}

int RE_TextureImporter::GetExtensionIL(const char * extension)
{
	if (std::strcmp(extension,"dds") == 0 || std::strcmp(extension, "eye") == 0)
		return IL_DDS;
	else if (std::strcmp(extension, "png") == 0)
		return IL_PNG;
	else if (std::strcmp(extension, "jpg") == 0)
		return IL_JPG;
	else if (std::strcmp(extension, "tga") == 0)
		return IL_TGA;
	return -1;
}