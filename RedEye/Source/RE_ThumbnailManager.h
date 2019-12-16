#ifndef __RETHUMBNAILMANAGER_H__
#define __RETHUMBNAILMANAGER_H__

#include <map>

class RE_CompCamera;

class RE_ThumbnailManager
{
public:
	RE_ThumbnailManager();
	~RE_ThumbnailManager();

	void Init();

	void Add(const char* ref);
	void Change(const char* ref);

	unsigned int At(const char* ref);

	unsigned int GetFolderID()const { return folder; }
	unsigned int GetFileID()const { return file; }
	unsigned int GetSelectFileID()const { return selectfile; }
	unsigned int GetShaderFileID()const { return shaderFile; }

private:
	unsigned int LoadDefIcon(const char* filename);


	unsigned int ThumbnailTexture(const char* ref);
	unsigned int ThumbnailGameObject(const char* ref);
	unsigned int LoadLibraryTexThumbnail(const char* ref);

private:
	std::map<const char*, unsigned int> thumbnails;

	unsigned int singleRenderFBO = 0;
	unsigned int pboRender = 0;
	RE_CompCamera* internalCamera = nullptr;

	unsigned int folder = 0;
	unsigned int file = 0;
	unsigned int selectfile = 0;
	unsigned int shaderFile = 0;
};

#endif // !__RETHUMBNAILMANAGER_H__