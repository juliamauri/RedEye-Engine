#ifndef __RE_THUMBNAIL_MANAGER_H__
#define __RE_THUMBNAIL_MANAGER_H__

#include <EASTL/map.h>

namespace RE_ThumbnailManager
{
	void Init();
	void Clear();

	void Change(const char* ref, unsigned int id);
	void Delete(const char* ref);

	unsigned int At(const char* ref);

	unsigned int GetFolderID();
	unsigned int GetFileID();
	unsigned int GetSelectFileID();
	unsigned int GetShaderFileID();

	unsigned int ThumbnailTexture(const char* ref);

	void SaveTextureFromFBO(const char* path);
	unsigned int LoadLibraryThumbnail(const char* ref);

	namespace Internal
	{
		unsigned int LoadDefIcon(const char* filename);

		static unsigned int thumbnail_size = 256;
		static unsigned int thumbnail_data_size = 0;

		static unsigned int singleRenderFBO = 0;
		static unsigned int pboRender = 0;
		static unsigned int folder = 0;
		static unsigned int file = 0;
		static unsigned int selectfile = 0;
		static unsigned int shaderFile = 0;

		static eastl::map<const char*, unsigned int> thumbnails;

		static const char* library_path = "Library/Thumbnails/";
		static const char* defaults = "Settings/Icons/";
	}
}

#endif // !__RE_THUMBNAIL_MANAGER_H__