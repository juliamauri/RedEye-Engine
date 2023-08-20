#ifndef __RETHUMBNAILMANAGER_H__
#define __RETHUMBNAILMANAGER_H__

#include <EASTL/map.h>

#define THUMBNAILPATH "Library/Thumbnails/"
#define DEFTHUMBNAILS "Internal/Icons/"
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

	uintptr_t At(const char* ref);

	uintptr_t GetFolderID()const { return folder; }
	uintptr_t GetFileID()const { return file; }
	uintptr_t GetSelectFileID()const { return selectfile; }
	uintptr_t GetShaderFileID()const { return shaderFile; }
	uintptr_t GetPEmitterFileID()const { return p_emitter; }
	uintptr_t GetPEmissionFileID()const { return p_emission; }
	uintptr_t GetPRenderFileID()const { return p_render; }

	uint32_t ThumbnailTexture(const char* ref);

	void SaveTextureFromFBO(const char* path);
	uint32_t LoadLibraryThumbnail(const char* ref);

private:

	uint32_t LoadDefIcon(const char* filename);

private:

	eastl::map<const char*, uint32_t> thumbnails;

	uint32_t singleRenderFBO = 0;
	uint32_t pboRender = 0;

	uint32_t folder = 0;
	uint32_t file = 0;
	uint32_t selectfile = 0;
	uint32_t shaderFile = 0;
	uint32_t p_emitter = 0, p_emission = 0, p_render = 0;
};

#endif // !__RETHUMBNAILMANAGER_H__