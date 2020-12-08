#ifndef __RE_FILE_H__
#define __RE_FILE_H__

#include <EASTL\string.h>

class RE_FileBuffer
{
public:
	RE_FileBuffer(const char* file_name, const char* from_zip = nullptr);
	virtual ~RE_FileBuffer();

	void Delete();

	void ClearBuffer();
	char* GetBuffer();
	const char* GetBuffer() const;

	virtual bool Load();
	virtual void Save();
	virtual void Save(char* buffer, unsigned int size = 0);

	virtual unsigned int GetSize();
	virtual eastl::string GetMd5();

	virtual inline bool operator!() const;

protected:

	unsigned int HardLoad();
	void HardSave(const char* buffer);
	void WriteFile(const char* zip_path, const char* filename, const char* buffer, unsigned int size);

protected:

	unsigned int size = 0;
	const char *from_zip, *file_name;
	char* buffer = nullptr;
};

#endif // !__RE_FILE_H__