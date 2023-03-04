#ifndef RL_FILESYSTEM_CLASS
#define RL_FILESYSTEM_CLASS

#include <EASTL/vector.h>
#include <EASTL/string.h>

class RL_FileSystem
{
public:
	bool Init(char* argv[]);
	void CleanUp();

	eastl::vector<eastl::string> GetFilespathFrom(const char* path);
	eastl::vector<eastl::string> GetFilesnameFrom(const char* path);

	bool Exist(const char* path)const;
	 
	void NewDir(const char* dir);
	void Write(const char* path, const char* file, const char* buff, unsigned int buff_size);
	void WriteOutside(const char* path, const char* file, const char* buff, unsigned int buff_size);
	eastl::string Read(const char* filepath);
	eastl::string ReadOutside(const char* filepath);

	const char* GetExecutableDirectory() const;
	const char* GetPrefDirectory() const;

private:
	eastl::string _exec_directory, _pref_directory;
};

#endif //RL_FILESYSTEM_CLASS