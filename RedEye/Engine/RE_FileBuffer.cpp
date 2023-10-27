#include "RE_FileBuffer.h"

#include "Application.h"
#include "RE_Memory.h"
#include <MD5/md5.h>
#include <PhysFS/physfs.h>
#include <EASTL/bit.h>

RE_FileBuffer::RE_FileBuffer(const char* file_name) : buffer(nullptr), file_name(file_name) {}
RE_FileBuffer::~RE_FileBuffer() { if (buffer != nullptr) delete[] buffer; }

bool RE_FileBuffer::Load()
{
	size = HardLoad();
	return size > 0;
}

void RE_FileBuffer::Save() { HardSave(buffer); }
void RE_FileBuffer::Save(char* buffer, size_t size) { HardSave(buffer, size); }

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
size_t RE_FileBuffer::GetSize() { return size; }
eastl::string RE_FileBuffer::GetMd5() { return (!buffer && !Load()) ? "" : md5(eastl::string(buffer, size)); }

size_t RE_FileBuffer::HardLoad()
{
	if (!PHYSFS_exists(file_name))
	{
		RE_LOG_ERROR("File System error: HardLoading file %s doesn't exist.\nPHYSFS Error: %s", file_name, PHYSFS_getLastError());
		return 0;
	}

	PHYSFS_File* fs_file = PHYSFS_openRead(file_name);

	if (fs_file == nullptr)
	{
		RE_LOG_ERROR("File System error while opening file %s.\nPHYSFS Error: %s", file_name, PHYSFS_getLastError());
		return 0;
	}

	PHYSFS_sint64 amount_read = 0;
	auto sll_size = PHYSFS_fileLength(fs_file);

	if (sll_size > 0)
	{
		DEL_A(buffer);
		buffer = new char[sll_size + 1];

		amount_read = PHYSFS_read(fs_file, buffer, 1, (PHYSFS_uint32)sll_size);

		if (amount_read == sll_size)
		{
			buffer[amount_read] = '\0';
		}
		else
		{
			RE_LOG_ERROR("File System error while reading from file %s: %s", file_name, PHYSFS_getLastError());
			DEL_A(buffer);
		}
	}

	if (!PHYSFS_close(fs_file))
		RE_LOG_ERROR("File System error while closing file %s: %s", file_name, PHYSFS_getLastError());


	return static_cast<size_t>(amount_read);
}

void RE_FileBuffer::HardSave(const char* buffer, size_t size)
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
		auto written = PHYSFS_write(file, eastl::bit_cast<const void*>(buffer), 1, static_cast<PHYSFS_uint32>((size == 0) ? size = (strnlen_s(buffer, 0xffff)) : size));
		if (written != size) RE_LOG_ERROR("Error while writing to file %s: %s", file, PHYSFS_getLastError());
		if (PHYSFS_close(file) == 0) RE_LOG_ERROR("Error while closing save file %s: %s", file, PHYSFS_getLastError());
	}
	else RE_LOG_ERROR("Error while opening save file %s: %s", file, PHYSFS_getLastError());
}