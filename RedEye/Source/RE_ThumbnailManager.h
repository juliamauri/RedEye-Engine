#ifndef __RETHUMBNAILMANAGER_H__
#define __RETHUMBNAILMANAGER_H__

#include <EASTL/map.h>

#define THUMBNAILPATH "Library/Thumbnails/"
#define DEFTHUMBNAILS "Settings/Icons/"
#define THUMBNAILSIZE 256
#define THUMBNAILDATASIZE THUMBNAILSIZE * THUMBNAILSIZE * 4

class RE_CompCamera;

class RE_ThumbnailManager
{
public:
	RE_ThumbnailManager() {}
	~RE_ThumbnailManager() {}

	void Init();
	void Clear();

	void Change(const char* ref, unsigned int id);
	void Delete(const char* ref);

	unsigned int At(const char* ref);

	unsigned int GetFolderID()const { return folder; }
	unsigned int GetFileID()const { return file; }
	unsigned int GetSelectFileID()const { return selectfile; }
	unsigned int GetShaderFileID()const { return shaderFile; }

	unsigned int ThumbnailTexture(const char* ref);

	void SaveTextureFromFBO(const char* path);
	unsigned int LoadLibraryThumbnail(const char* ref);

private:

	unsigned int LoadDefIcon(const char* filename);

private:

	eastl::map<const char*, unsigned int> thumbnails;

	unsigned int singleRenderFBO = 0;
	unsigned int pboRender = 0;

	unsigned int folder = 0;
	unsigned int file = 0;
	unsigned int selectfile = 0;
	unsigned int shaderFile = 0;
};

#endif // !__RETHUMBNAILMANAGER_H__