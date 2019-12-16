#include "RE_ThumbnailManager.h"

#include "Application.h"
#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"
#include "RE_TextureImporter.h"
#include "RE_GLCache.h"

#include "RE_Texture.h"

#include "Glew/include/glew.h"

#include "IL/include/il.h"
#include "IL/include/ilu.h"

#include "Globals.h"

#include <string>

#define THUMBNAILPATH "Library/Thumbnails/"

#define DEFTHUMBNAILS "Settings/Icons/"

#define THUMBNAILSIZE 256

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
		thumbnails.insert(std::pair<const char*, unsigned int>(ref, ThumbnailTexture(ref)));
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
	uint ret = 0;
	std::string path(DEFTHUMBNAILS);
	path += filename;
	RE_FileIO filderIcon(path.c_str());
	if (filderIcon.Load()) {
		RE_TextureSettings defTexSettings;
		int tmp1, tmp2;
		App->textures->LoadTextureInMemory(filderIcon.GetBuffer(), filderIcon.GetSize(), TextureType::RE_DDS, &ret, &tmp1, &tmp2, defTexSettings);
	}
	return ret;
}

unsigned int RE_ThumbnailManager::ThumbnailTexture(const char* ref)
{
	uint ret = 0;

	std::string path(THUMBNAILPATH);
	path += ref;

	if (!App->fs->Exists(path.c_str())) {
		ResourceContainer* res = App->resources->At(ref);
		RE_Texture* tex = (RE_Texture*)res;
		RE_FileIO texFile(res->GetAssetPath());
		if (texFile.Load());
		{
			unsigned int imageID = 0;
			ilGenImages(1, &imageID);
			ilBindImage(imageID);

			if (IL_FALSE != ilLoadL(tex->DetectExtension(), texFile.GetBuffer(), texFile.GetSize())) {

				//ILubyte* bytes = ilGetData();
				//ilTexImage(
				//
				//	THUMBNAILSIZE,
				//	THUMBNAILSIZE,
				//
				//	1,  // OpenIL supports 3d textures!  but we don't want it to be 3d.  so
				//	// we just set this to be 1
				//
				//	3,  // 3 channels:  one for R , one for G, one for B
				//
				//	TextureType::RE_DDS,  // duh, yeah use rgb!  coulda been rgba if we wanted trans
				//
				//	IL_UNSIGNED_BYTE,  // the type of data the imData array contains (next)
				//
				//	bytes  // and the array of bytes represneting the actual image data
				//
				//);
				iluScale(THUMBNAILSIZE, THUMBNAILSIZE, 1);

				RE_FileIO saveThumb(path.c_str(), App->fs->GetZipPath());

				ILuint   size = ilSaveL(IL_DDS, NULL, 0); // Get the size of the data buffer
				ILubyte* data = new ILubyte[size];

				ilSaveL(IL_DDS, data, size); // Save with the ilSaveIL function
				saveThumb.Save((char*)data, size);
				DEL_A(data);
				
				ilBindImage(0);
				/* Delete used resources*/
				ilDeleteImages(1, &imageID); /* Because we have already copied image data into texture data we can release memory used by image. */
			}
		}
	}
	RE_FileIO thumbFile(path.c_str());
	if(thumbFile.Load()) {
		RE_TextureSettings defSettings;
		uint imageID = 0;
		ilGenImages(1, &imageID);
		ilBindImage(imageID);

		if (IL_FALSE != ilLoadL(TextureType::RE_DDS, thumbFile.GetBuffer(), thumbFile.GetSize())) {


			/* OpenGL texture binding of the image loaded by DevIL  */
			glGenTextures(1, &ret); /* Texture name generation */
			RE_GLCache::ChangeTextureBind(ret); /* Binding of texture name */

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, defSettings.mag_filter); /* We will use linear interpolation for magnification filter */
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, defSettings.min_filter); /* We will use linear interpolation for minifying filter */
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, defSettings.wrap_s); /* We will use linear interpolation for minifying filter */
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, defSettings.wrap_t); /* We will use linear interpolation for minifying filter */

			glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData()); /* Texture specification */

			ilBindImage(0);
			/* Delete used resources*/
			ilDeleteImages(1, &imageID); /* Because we have already copied image data into texture data we can release memory used by image. */
		}
	}

	return ret;
}
