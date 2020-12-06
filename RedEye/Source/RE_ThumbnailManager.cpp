#include "RE_ThumbnailManager.h"

#include "Application.h"
#include "RE_FileSystem.h"
#include "ModuleWindow.h"
#include "RE_ResourceManager.h"
#include "RE_ShaderImporter.h"
#include "RE_InternalResources.h"
#include "RE_TextureImporter.h"
#include "ModuleScene.h"
#include "RE_GLCacheManager.h"
#include "RE_FBOManager.h"
#include "Event.h"
#include "RE_TimeManager.h"
#include "RE_ECS_Manager.h"
#include "RE_Shader.h"
#include "RE_Texture.h"
#include "RE_Model.h"

#include "RE_Material.h"
#include "RE_SkyBox.h"
#include "Globals.h"

#include "Glew/include/glew.h"
#include "IL/include/il.h"
#include "IL/include/ilu.h"
#include "par_shapes.h"
#include <EASTL/string.h>

RE_ThumbnailManager::RE_ThumbnailManager() {}

RE_ThumbnailManager::~RE_ThumbnailManager()
{
	glDeleteTextures(1, &folder);
	glDeleteTextures(1, &file);
	glDeleteTextures(1, &selectfile);
	glDeleteTextures(1, &shaderFile);
	for (auto thumb : thumbnails) glDeleteTextures(1, &thumb.second);

	glDeleteBuffersARB(1, &pboRender);
}

void RE_ThumbnailManager::Init()
{
	folder = LoadDefIcon("folder.dds");
	file = LoadDefIcon("file.dds");
	selectfile = LoadDefIcon("selectfile.dds");
	shaderFile = LoadDefIcon("shaderFile.dds");
	
	glGenBuffersARB(1, &pboRender);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboRender);
	glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, THUMBNAILDATASIZE, 0, GL_STREAM_READ_ARB);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
}

void RE_ThumbnailManager::Change(const char* ref, unsigned int id)
{
	auto iter = thumbnails.find(ref);
	if (iter == thumbnails.end()) thumbnails.insert(eastl::pair<const char*, unsigned int>(ref, id));
	else iter->second = id;
}

void RE_ThumbnailManager::Delete(const char* ref)
{
	thumbnails.erase(ref);
	eastl::string path(THUMBNAILPATH);
	if (App::fs->Exists((path += ref).c_str()))
	{
		RE_FileIO fileToDelete(path.c_str(), App::fs->GetZipPath());
		fileToDelete.Delete();
	}
}

unsigned int RE_ThumbnailManager::At(const char* ref) { 

	return (thumbnails.find(ref) != thumbnails.end()) ? thumbnails.at(ref) : 0;

}

unsigned int RE_ThumbnailManager::LoadDefIcon(const char* filename)
{
	uint ret = 0;
	eastl::string path(DEFTHUMBNAILS);
	path += filename;
	RE_FileIO filderIcon(path.c_str());
	if (filderIcon.Load())
	{
		RE_TextureSettings defTexSettings;
		int tmp1, tmp2;
		App::textures.LoadTextureInMemory(filderIcon.GetBuffer(), filderIcon.GetSize(), TextureType::RE_DDS, &ret, &tmp1, &tmp2, defTexSettings);
	}
	return ret;
}

unsigned int RE_ThumbnailManager::ThumbnailTexture(const char* ref)
{
	uint ret = 0;
	eastl::string path(THUMBNAILPATH);
	if (!App::fs->Exists((path += ref).c_str()))
	{
		ResourceContainer* res = App::resources->At(ref);
		RE_Texture* tex = dynamic_cast<RE_Texture*>(res);
		RE_FileIO texFile(res->GetAssetPath());
		if (texFile.Load())
		{
			unsigned int imageID = 0;
			ilGenImages(1, &imageID);
			ilBindImage(imageID);

			if (IL_FALSE != ilLoadL(tex->DetectExtension(), texFile.GetBuffer(), texFile.GetSize()))
			{
				iluScale(THUMBNAILSIZE, THUMBNAILSIZE, 1);
				RE_FileIO saveThumb(path.c_str(), App::fs->GetZipPath());
				ILuint   size = ilSaveL(IL_DDS, NULL, 0); // Get the size of the data buffer
				ILubyte* data = new ILubyte[size];
				ilSaveL(IL_DDS, data, size); // Save with the ilSaveIL function
				saveThumb.Save(reinterpret_cast<char*>(data), size);
				DEL_A(data);	
			}

			ilBindImage(0);
			/* Delete used resources*/
			ilDeleteImages(1, &imageID); /* Because we have already copied image data into texture data we can release memory used by image. */
		}
	}

	ret = LoadLibraryThumbnail(ref);

	return ret;
}

void RE_ThumbnailManager::SaveTextureFromFBO(const char* path)
{
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboRender);
	glReadPixels(0, 0, THUMBNAILSIZE, THUMBNAILSIZE, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	GLubyte* ptr = static_cast<GLubyte*>(glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
	if (ptr)
	{
		uint imageID = 0;
		ilGenImages(1, &imageID);
		ilBindImage(imageID);
		ilTexImage(THUMBNAILSIZE, THUMBNAILSIZE, 1, 4, IL_RGBA, GL_UNSIGNED_BYTE, ptr);

		ILuint   size = ilSaveL(IL_DDS, NULL, 0); // Get the size of the data buffer
		ILubyte* data = new ILubyte[size];

		ilSaveL(IL_DDS, data, size); // Save with the ilSaveIL function
		RE_FileIO saveThumb(path, App::fs->GetZipPath());
		saveThumb.Save(reinterpret_cast<char*>(data), size);
		DEL_A(data);

		ilBindImage(0);
		/* Delete used resources*/
		ilDeleteImages(1, &imageID); /* Because we have already copied image data into texture data we can release memory used by image. */

		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	}
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
}

unsigned int RE_ThumbnailManager::LoadLibraryThumbnail(const char* ref)
{
	uint ret = 0;
	eastl::string path(THUMBNAILPATH);
	RE_FileIO thumbFile((path += ref).c_str());
	if (thumbFile.Load())
	{
		RE_TextureSettings defSettings;
		uint imageID = 0;
		ilGenImages(1, &imageID);
		ilBindImage(imageID);

		if (IL_FALSE != ilLoadL(TextureType::RE_DDS, thumbFile.GetBuffer(), thumbFile.GetSize()))
		{
			/* OpenGL texture binding of the image loaded by DevIL  */
			glGenTextures(1, &ret); /* Texture name generation */
			RE_GLCacheManager::ChangeTextureBind(ret); /* Binding of texture name */

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, defSettings.mag_filter); /* We will use linear interpolation for magnification filter */
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, defSettings.min_filter); /* We will use linear interpolation for minifying filter */
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, defSettings.wrap_s); /* We will use linear interpolation for minifying filter */
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, defSettings.wrap_t); /* We will use linear interpolation for minifying filter */

			glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), THUMBNAILSIZE, THUMBNAILSIZE, 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData()); /* Texture specification */

			RE_GLCacheManager::ChangeTextureBind(0);
			ilBindImage(0);
			/* Delete used resources*/
			ilDeleteImages(1, &imageID); /* Because we have already copied image data into texture data we can release memory used by image. */
		}
	}
	return ret;
}