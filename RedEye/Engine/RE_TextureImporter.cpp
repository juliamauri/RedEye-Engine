#include "RE_TextureImporter.h"

#include "RE_Memory.h"
#include "RE_Profiler.h"
#include "Application.h"
#include "RE_FileSystem.h"
#include "RE_FileBuffer.h"
#include "RE_ResourceManager.h"
#include "RE_GLCache.h"
#include "RE_Texture.h"

#include <GL/Glew/glew.h>
#include "IL\include\il.h"
#include "IL\include\ilu.h"
#include "IL\include\ilut.h"
#include <EAStdC\EASprintf.h>

#pragma comment(lib, "IL/libx86/DevIL.lib")
#pragma comment(lib, "IL/libx86/ILU.lib")
#pragma comment(lib, "IL/libx86/ILUT.lib")

bool RE_TextureImporter::Init()
{
	RE_PROFILE(PROF_Init, PROF_TextureImporter);
	RE_LOG("Initializing Texture Importer");
	ilInit();
	iluInit();
	ilutInit();

	ILenum error = ilGetError();
	if (error == IL_NO_ERROR) {
		if (ilutRenderer(ILUT_OPENGL)) {
			if (ilOriginFunc(IL_ORIGIN_LOWER_LEFT)) {
				if (ilEnable(IL_ORIGIN_SET)) {
					char tmp[8];
					EA::StdC::Snprintf(tmp, 8, "%u.%u.%u", IL_VERSION / 100, (IL_VERSION % 100) / 10, IL_VERSION % 10);
					RE_SOFT_NVS("DevIL", tmp, "http://openil.sourceforge.net/");
					return true;
				} else	RE_LOG_ERROR("DevIL Init Error when enabling origin IL_ORIGIN_SET");
			} else		RE_LOG_ERROR("DevIL Init Error when setting origin to IL_ORIGIN_LOWER_LEFT");
		} else			RE_LOG_ERROR("DevIL Init Error when setting renderer to ILUT_OPENGL");
	} else				RE_LOG_ERROR("DevIL Init Error %d - %s", error, iluErrorString(error));

	return false;
}

const char * RE_TextureImporter::AddNewTextureOnResources(const char * assetsPath)
{
	const char* retMD5 = nullptr;
	eastl::string path(assetsPath);
	eastl::string filename = path.substr(path.find_last_of("/") + 1);
	eastl::string name = filename.substr(0, filename.find_last_of(".") - 1);
	eastl::string extension = filename.substr(filename.find_last_of(".") + 1);

	RE_Texture* newTexture = new RE_Texture();
	newTexture->SetAssetPath(path.c_str());
	if (newTexture->DetectExtension() != RE_TEXTURE_UNKNOWN)
	{
		newTexture->SetName(name.c_str());
		newTexture->SetType(Resource_Type::R_TEXTURE);
		retMD5 = newTexture->GenerateMD5();
		newTexture->SaveMeta();
	}
	else
		RE_LOG_ERROR("Error detecting texture extension. Don't suported %s", extension.c_str());

	return retMD5;
}

const char* RE_TextureImporter::TransformToDDS(const char* assetBuffer, unsigned int assetSize, TextureType assetType, unsigned int* newSize)
{
	eastl::string ret;
	unsigned int imageID = 0;
	ilGenImages(1, &imageID);
	ilBindImage(imageID);

	if (IL_FALSE != ilLoadL(assetType, assetBuffer, assetSize)) {
		if (assetType == IL_TGA)
			ilFlipSurfaceDxtcData();

		//Save into dds
		*newSize = ilSaveL(IL_DDS, NULL, 0); // Get the size of the data buffer
		ILubyte* data = new ILubyte[*newSize];

		ilSaveL(IL_DDS, data, *newSize); // Save with the ilSaveIL function
		ret = eastl::string((char*)data, *newSize);
		ilBindImage(0);
		/* Delete used resources*/
		ilDeleteImages(1, &imageID); /* Because we have already copied image data into texture data we can release memory used by image. */
	} else {
		RE_LOG_ERROR("Error when loading texture on DevIL");
	}

	return ret.c_str();
}

void RE_TextureImporter::LoadTextureInMemory(const char * buffer, unsigned int size, TextureType type, unsigned int * ID, int * width, int * height, RE_TextureSettings settings)
{
	unsigned int imageID = 0;
	ilGenImages(1, &imageID);
	ilBindImage(imageID);

	if (IL_FALSE != ilLoadL(type, buffer, size)) {

		if (type == IL_TGA)
			iluFlipImage();

		/* OpenGL texture binding of the image loaded by DevIL  */
		glGenTextures(1, ID); /* Texture name generation */
		RE_GLCache::ChangeTextureBind(*ID); /* Binding of texture name */
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, settings.mag_filter); /* We will use linear interpolation for magnification filter */
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, settings.min_filter); /* We will use linear interpolation for minifying filter */
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, settings.wrap_s); /* We will use linear interpolation for minifying filter */
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, settings.wrap_t); /* We will use linear interpolation for minifying filter */

		if(settings.wrap_s == RE_TextureWrap::RE_CLAMP_TO_BORDER || settings.wrap_t == RE_TextureWrap::RE_CLAMP_TO_BORDER)
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &settings.borderColor[0]);

		glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), *width = ilGetInteger(IL_IMAGE_WIDTH), *height = ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData()); /* Texture specification */
		if(settings.min_filter >= RE_TextureFilters::RE_NEAREST_MIPMAP_NEAREST)
			glGenerateMipmap(GL_TEXTURE_2D);

		ilBindImage(0);
		/* Delete used resources*/
		ilDeleteImages(1, &imageID); /* Because we have already copied image data into texture data we can release memory used by image. */
	}
	else {
		RE_LOG_ERROR("Error when loading texture on DevIL");
	}
}

void RE_TextureImporter::SaveOwnFormat(const char* assetBuffer, unsigned int assetSize, TextureType assetType, RE_FileBuffer* toSave)
{
	unsigned int imageID = 0;
	ilGenImages(1, &imageID);
	ilBindImage(imageID);

	if (IL_FALSE != ilLoadL(assetType, assetBuffer, assetSize))
	{
		if (assetType == IL_TGA) ilFlipSurfaceDxtcData();

		//Save into dds
		ILuint   size = ilSaveL(IL_DDS, NULL, 0); // Get the size of the data buffer
		ILubyte *data = new ILubyte[size];
		ilSaveL(IL_DDS, data, size); // Save with the ilSaveIL function
		toSave->Save((char*)data, size);
		DEL_A(data);
		ilBindImage(0);
		/* Delete used resources*/
		ilDeleteImages(1, &imageID); /* Because we have already copied image data into texture data we can release memory used by image. */
	}
	else RE_LOG_ERROR("Error when loading texture on DevIL");
}