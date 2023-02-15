#include "RE_FileBuffer.h"

#include "Application.h"
#include <MD5/md5.h>
#include <PhysFS/physfs.h>
#include <libzip/zip.h>

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

/* Write
void RE_FileBuffer::WriteFile(const char* zip_path, const char* filename, const char* buffer, unsigned int size)
{
	if (PHYSFS_removeFromSearchPath(from_zip) == 0)
		RE_LOG_ERROR("Ettot when unmount: %s", PHYSFS_getLastError());

	struct zip* f_zip = NULL;
	int error = 0;
	f_zip = zip_open(zip_path, ZIP_CHECKCONS, &error); // on ouvre l'archive zip 
	if (error)	RE_LOG_ERROR("could not open or create archive: %s", zip_path);

	zip_source_t* s = zip_source_buffer(f_zip, buffer, size, 0);
	if (s == NULL || zip_file_add(f_zip, filename, s, ZIP_FL_OVERWRITE + ZIP_FL_ENC_UTF_8) < 0)
	{
		zip_source_free(s);
		RE_LOG_ERROR("error adding file: %s\n", zip_strerror(f_zip));
	}

	zip_close(f_zip);
	f_zip = NULL;
	PHYSFS_mount(from_zip, NULL, 1);
}
*/

/* Delete
if (PHYSFS_removeFromSearchPath(from_zip) == 0)
	RE_LOG_ERROR("Ettot when unmount: %s", PHYSFS_getLastError());

struct zip* f_zip = NULL;
int error = 0;
f_zip = zip_open(from_zip, ZIP_CHECKCONS, &error); //on ouvre l'archive zip
if (error)	RE_LOG_ERROR("could not open or create archive: %s", from_zip);

zip_int64_t index = zip_name_locate(f_zip, file_name, NULL);
if (index == -1) RE_LOG_ERROR("file culdn't locate: %s", file_name);
else if (zip_delete(f_zip, index) == -1) RE_LOG_ERROR("file culdn't delete: %s", file_name);

zip_close(f_zip);
f_zip = NULL;

PHYSFS_mount(from_zip, NULL, 1);
*/