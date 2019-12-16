#ifndef __RETHUMBNAILMANAGER_H__
#define __RETHUMBNAILMANAGER_H__

#include <map>

class RE_ThumbnailManager
{
public:
	RE_ThumbnailManager();
	~RE_ThumbnailManager();

	void Init();

	void Add(const char* ref);

	unsigned int At(const char* ref)const;

	unsigned int GetFolderID()const { return folder; }
	unsigned int GetFileID()const { return file; }
	unsigned int GetSelectFileID()const { return selectfile; }
	unsigned int GetShaderFileID()const { return shaderFile; }

private:
	unsigned int LoadDefIcon(const char* filename);

private:
	std::map<const char*, unsigned int> thumbnails;

	unsigned int folder = 0;
	unsigned int file = 0;
	unsigned int selectfile = 0;
	unsigned int shaderFile = 0;
};

#endif // !__RETHUMBNAILMANAGER_H__