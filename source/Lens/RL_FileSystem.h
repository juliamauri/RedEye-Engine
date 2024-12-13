#ifndef RL_FILESYSTEM_CLASS
#define RL_FILESYSTEM_CLASS

#include <vector>
#include <string>

class RL_FileSystem
{
public:
	bool Init(char* argv[]);
	void CleanUp();

	std::vector<std::string> GetFilespathFrom(const char* path);
	std::vector<std::string> GetFilesnameFrom(const char* path);

	bool Exist(const char* path)const;
	 
	void NewDir(const char* dir);
	void Write(const char* path, const char* file, const char* buff, unsigned int buff_size);
	void WriteOutside(const char* path, const char* file, const char* buff, unsigned int buff_size);
	std::string Read(const char* filepath);
	std::string ReadOutside(const char* filepath);

	const char* GetExecutableDirectory() const;
	const char* GetPrefDirectory() const;

private:
	std::string _exec_directory, _pref_directory;
};

#endif //RL_FILESYSTEM_CLASS