#ifndef __RE_FILE_H__
#define __RE_FILE_H__

#include <EASTL\string.h>

class RE_FileBuffer
{
public:
	RE_FileBuffer(const char* file_name);
	virtual ~RE_FileBuffer();

	void Delete();

	void ClearBuffer();
	char* GetBuffer();
	const char* GetBuffer() const;

	virtual bool Load();
	virtual void Save();
	virtual void Save(char* buffer, size_t size = 0);

	virtual size_t GetSize();
	virtual eastl::string GetMd5();

	virtual inline bool operator!() const;

protected:

	size_t HardLoad();
	void HardSave(const char* buffer, size_t size = 0);

protected:

	size_t size = 0;
	const char *file_name;
	char* buffer = nullptr;
};

#endif // !__RE_FILE_H__