#include "RE_ThumbnailManager.h"

#include "RE_Memory.h"
#include "RE_DataTypes.h"
#include "Application.h"
#include "RE_FileSystem.h"
#include "RE_FileBuffer.h"
#include "RE_TextureImporter.h"
#include "RE_ResourceManager.h"
#include "RE_GLCache.h"
#include "RE_Texture.h"

#include <GL/glew.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <EASTL/string.h>

void RE_ThumbnailManager::Init()
{
	folder = LoadDefIcon("folder.dds");
	file = LoadDefIcon("file.dds");
	selectfile = LoadDefIcon("selectfile.dds");
	shaderFile = LoadDefIcon("shaderFile.dds");
	p_emitter = LoadDefIcon("pemitter.dds");
	p_emission = LoadDefIcon("pemissor.dds");
	p_render = LoadDefIcon("prender.dds");
	
	glGenBuffersARB(1, &pboRender);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboRender);
	glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, THUMBNAILDATASIZE, 0, GL_STREAM_READ_ARB);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
}

void RE_ThumbnailManager::Clear()
{
	
	glDeleteTextures(1, &folder);
	glDeleteTextures(1, &file);
	glDeleteTextures(1, &selectfile);
	glDeleteTextures(1, &shaderFile);
	for (auto thumb : thumbnails) glDeleteTextures(1, &thumb.second);

	glDeleteBuffersARB(1, &pboRender);
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
	if (RE_FS->Exists((path += ref).c_str()))
	{
		RE_FileBuffer fileToDelete(path.c_str());
		fileToDelete.Delete();
	}
}

uintptr_t RE_ThumbnailManager::At(const char* ref)
{ 
	return (thumbnails.find(ref) != thumbnails.end()) ? thumbnails.at(ref) : 0u;
}

uint32_t RE_ThumbnailManager::LoadDefIcon(const char* filename)
{
	unsigned int ret = 0u;
	eastl::string path(DEFTHUMBNAILS);
	path += filename;
	RE_FileBuffer filderIcon(path.c_str());
	if (filderIcon.Load())
	{
		RE_TextureSettings defTexSettings;
		int tmp1, tmp2;
		RE_TextureImporter::LoadTextureInMemory(filderIcon.GetBuffer(), filderIcon.GetSize(), TextureType::RE_DDS, &ret, &tmp1, &tmp2, defTexSettings);
	}
	return ret;
}

uint32_t RE_ThumbnailManager::ThumbnailTexture(const char* ref)
{
	unsigned int ret = 0u;
	eastl::string path(THUMBNAILPATH);
	if (!RE_FS->Exists((path += ref).c_str()))
	{
		ResourceContainer* res = RE_RES->At(ref);
		RE_Texture* tex = dynamic_cast<RE_Texture*>(res);
		RE_FileBuffer texFile(res->GetAssetPath());
		if (texFile.Load())
		{
			unsigned int imageID = 0;
			ilGenImages(1, &imageID);
			ilBindImage(imageID);

			if (IL_FALSE != ilLoadL(static_cast<ILenum>(tex->DetectExtension()), texFile.GetBuffer(), static_cast<ILuint>(texFile.GetSize())))
			{
				iluScale(THUMBNAILSIZE, THUMBNAILSIZE, 1);
				RE_FileBuffer saveThumb(path.c_str());
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
		RE_FileBuffer saveThumb(path);
		saveThumb.Save(reinterpret_cast<char*>(data), size);
		DEL_A(data);

		ilBindImage(0);
		/* Delete used resources*/
		ilDeleteImages(1, &imageID); /* Because we have already copied image data into texture data we can release memory used by image. */

		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	}
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
}

uint32_t RE_ThumbnailManager::LoadLibraryThumbnail(const char* ref)
{
	uint ret = 0u;
	eastl::string path(THUMBNAILPATH);
	RE_FileBuffer thumbFile((path += ref).c_str());
	if (thumbFile.Load())
	{
		RE_TextureSettings defSettings;
		uint imageID = 0;
		ilGenImages(1, &imageID);
		ilBindImage(imageID);

		if (IL_FALSE != ilLoadL(TextureType::RE_DDS, thumbFile.GetBuffer(), static_cast<ILuint>(thumbFile.GetSize())))
		{
			/* OpenGL texture binding of the image loaded by DevIL  */
			glGenTextures(1, &ret); /* Texture name generation */
			RE_GLCache::ChangeTextureBind(ret); /* Binding of texture name */

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, defSettings.mag_filter); /* We will use linear interpolation for magnification filter */
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, defSettings.min_filter); /* We will use linear interpolation for minifying filter */
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, defSettings.wrap_s); /* We will use linear interpolation for minifying filter */
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, defSettings.wrap_t); /* We will use linear interpolation for minifying filter */

			glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), THUMBNAILSIZE, THUMBNAILSIZE, 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData()); /* Texture specification */

			RE_GLCache::ChangeTextureBind(0);
			ilBindImage(0);
			/* Delete used resources*/
			ilDeleteImages(1u, &imageID); /* Because we have already copied image data into texture data we can release memory used by image. */
		}
	}
	return ret;
}