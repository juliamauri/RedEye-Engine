#ifndef __RETHUMBNAILMANAGER_H__
#define __RETHUMBNAILMANAGER_H__

#include <EASTL/map.h>

class RE_CompCamera;

class RE_ThumbnailManager
{
public:
	RE_ThumbnailManager();
	~RE_ThumbnailManager();

	void Init();

	void Add(const char* ref);
	void Change(const char* ref);
	void Delete(const char* ref);

	unsigned int At(const char* ref);

	unsigned int GetFolderID()const { return folder; }
	unsigned int GetFileID()const { return file; }
	unsigned int GetSelectFileID()const { return selectfile; }
	unsigned int GetShaderFileID()const { return shaderFile; }

private:
	unsigned int LoadDefIcon(const char* filename);


	unsigned int ThumbnailTexture(const char* ref);
	unsigned int ThumbnailGameObject(const char* ref);
	unsigned int ThumbnailMaterial(const char* ref);
	unsigned int ThumbnailSkyBox(const char* ref);
	void SaveTextureFromFBO(const char* path);
	unsigned int LoadLibraryThumbnail(const char* ref);

private:
	eastl::map<const char*, unsigned int> thumbnails;

	unsigned int singleRenderFBO = 0;
	unsigned int pboRender = 0;
	RE_CompCamera* internalCamera = nullptr;

	unsigned int folder = 0;
	unsigned int file = 0;
	unsigned int selectfile = 0;
	unsigned int shaderFile = 0;

	unsigned int VAOSphere = 0;
	unsigned int VBOSphere = 0;
	unsigned int EBOSphere = 0;
	unsigned int sphere_triangle_count = 0;
};

#endif // !__RETHUMBNAILMANAGER_H__