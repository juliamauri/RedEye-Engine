#include "RE_FileBuffer.h"

#include "Application.h"
#include <MD5/md5.h>
#include <PhysFS/physfs.h>

RE_FileBuffer::RE_FileBuffer(const char* file_name) : buffer(nullptr), file_name(file_name) {}
RE_FileBuffer::~RE_FileBuffer() { if (buffer != nullptr) delete[] buffer; }

bool RE_FileBuffer::Load()
{
	size = HardLoad();
	return size > 0;
}

void RE_FileBuffer::Save() { HardSave(buffer); }
void RE_FileBuffer::Save(char* buffer, unsigned int size) { HardSave(buffer, size); }

void RE_FileBuffer::Delete()
{
	if(PHYSFS_delete(file_name) == 0)
		RE_LOG_ERROR("File System error while deleting file %s: %s", file_name, PHYSFS_getLastError());
}

void RE_FileBuffer::ClearBuffer()
{
	delete[] buffer;
	buffer = nullptr;
}

char* RE_FileBuffer::GetBuffer() { return (buffer); }
const char* RE_FileBuffer::GetBuffer() const { return (buffer); }
inline bool RE_FileBuffer::operator!() const { return buffer == nullptr; }
unsigned int RE_FileBuffer::GetSize() { return size; }
eastl::string RE_FileBuffer::GetMd5() { return (!buffer && !Load()) ? "" : md5(eastl::string(buffer, size)); }

unsigned int RE_FileBuffer::HardLoad()
{
	unsigned int ret = 0u;

	if (PHYSFS_exists(file_name))
	{
		PHYSFS_File* fs_file = PHYSFS_openRead(file_name);

		if (fs_file != NULL)
		{
			signed long long sll_size = PHYSFS_fileLength(fs_file);
			if (sll_size > 0)
			{
				if (buffer != nullptr) delete[] buffer;
				buffer = new char[(unsigned int)sll_size + 1];
				signed long long amountRead = PHYSFS_read(fs_file, buffer, 1, (signed int)sll_size);

				if (amountRead != sll_size)
				{
					RE_LOG_ERROR("File System error while reading from file %s: %s", file_name, PHYSFS_getLastError());
					delete (buffer);
				}
				else
				{
					ret = (unsigned int)amountRead;
					buffer[ret] = '\0';
				}
			}

			if (PHYSFS_close(fs_file) == 0) RE_LOG_ERROR("File System error while closing file %s: %s", file_name, PHYSFS_getLastError());
		}
		else RE_LOG_ERROR("File System error while opening file %s: %s", file_name, PHYSFS_getLastError());
	}
	else RE_LOG_ERROR("File System error while checking file %s: %s", file_name, PHYSFS_getLastError());

	return ret;
}

void RE_FileBuffer::HardSave(const char* buffer, unsigned int size)
{
	std::string path(file_name);
	std::size_t botDirPos = path.find_last_of("/");
	// get directory
	std::string dir = path.substr(0, botDirPos);

	if (PHYSFS_exists(dir.c_str()) == 0)
		PHYSFS_mkdir(dir.c_str());

	PHYSFS_file* file = PHYSFS_openWrite(file_name);

	if (file != NULL)
	{
		long long written = PHYSFS_write(file, (const void*)buffer, 1, (size == 0) ? size = (strnlen_s(buffer, 0xffff)) : size);
		if (written != size) RE_LOG_ERROR("Error while writing to file %s: %s", file, PHYSFS_getLastError());
		if (PHYSFS_close(file) == 0) RE_LOG_ERROR("Error while closing save file %s: %s", file, PHYSFS_getLastError());
	}
	else RE_LOG_ERROR("Error while opening save file %s: %s", file, PHYSFS_getLastError());
}