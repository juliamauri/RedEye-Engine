#include "RE_SkyboxImporter.h"

#include "RE_FileSystem.h"
#include "RE_FileBuffer.h"
#include "Application.h"
#include "RE_GLCache.h"
#include "RE_ResourceManager.h"
#include "RE_Texture.h"

#include <GL/glew.h>
#include <IL/il.h>
#include <IL/ilu.h>

void RE_SkyboxImporter::LoadSkyBoxInMemory(RE_SkyBoxSettings& settings, unsigned int* ID, bool isDDS)
{
	RE_GLCache::ChangeTextureBind(0);
	glGenTextures(1, ID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, *ID);

	for (int i = 0; i < 6; i++)
	{
		const char* texPath = nullptr;
		if (!isDDS)
		{
			texPath = RE_RES->At(settings.textures[i].textureMD5)->GetLibraryPath();
			if (!RE_FS->Exists(texPath))
				RE_RES->At(settings.textures[i].textureMD5)->ReImport();
		}
		else texPath = settings.textures[i].path.c_str();

		RE_FileBuffer librayTexture(texPath);
		if (librayTexture.Load())
		{
			ILuint imageID = 0;
			ilGenImages(1, &imageID);
			ilBindImage(imageID);

			if (IL_FALSE != ilLoadL(
				static_cast<ILenum>(RE_TextureSettings::Type::DDS),
				librayTexture.GetBuffer(),
				static_cast<ILuint>(librayTexture.GetSize())))
			{
				iluFlipImage();

				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0,
					ilGetInteger(IL_IMAGE_BPP),
					ilGetInteger(IL_IMAGE_WIDTH),
					ilGetInteger(IL_IMAGE_HEIGHT),
					0,
					ilGetInteger(IL_IMAGE_FORMAT),
					GL_UNSIGNED_BYTE, ilGetData());

				ilBindImage(0);
				ilDeleteImages(1, &imageID);
			}
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, settings.min_filter);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, settings.mag_filter);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, settings.wrap_s);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, settings.wrap_t);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, settings.wrap_r);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
