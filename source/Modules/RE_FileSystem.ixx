module;

#include <physfs.h>

#include <vector>
#include <string>

export module FileSystem;

namespace {
	std::string _exec_directory;
	std::string _pref_directory;
}

export namespace RE {
	namespace FileSystem {
		bool Init(char* argv[], const char* org, const char* app)
		{
			if (PHYSFS_init(argv[0]) != 0) {

				_pref_directory = PHYSFS_getPrefDir(org, app);
				_exec_directory = argv[0];
				_exec_directory = _exec_directory.substr(0, _exec_directory.find_last_of('\\') + 1);

				if (PHYSFS_mount(".", 0, 0) == 0) {

					PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
					return false;
				}
				if (PHYSFS_mount(_pref_directory.c_str(), "WriteDir/", 0) == 0) {

					PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
					return false;
				}
				if (PHYSFS_setWriteDir(_pref_directory.c_str()) == 0) {

					PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
					return false;
				}
			}
			return true;
		}

		void CleanUp()
		{
			PHYSFS_deinit();
		}

		bool Exist(const char* path)
		{
			return PHYSFS_exists(path) != 0;
		}

		void NewDir(const char* dir)
		{
			if (PHYSFS_exists(dir) != 0) return;
			if (PHYSFS_mkdir(dir) == 0)
				PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
			//else already exists
		}

		void Write(const char* path, const char* file, const char* buff, unsigned int buff_size)
		{
			std::string filepath(std::string(path) + std::string(file));

			NewDir(path);

			PHYSFS_file* myfile = PHYSFS_openWrite(filepath.c_str());
			int length_writed = PHYSFS_write(myfile, buff, 1, buff_size);
			PHYSFS_close(myfile);
		}

		void WriteOutside(const char* path, const char* file, const char* buff, unsigned int buff_size)
		{
			if (PHYSFS_unmount(".") == 0) {
				PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
				int i = 0;
			}
			if (PHYSFS_mount(path, 0, 0) == 0) {
				PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
				int i = 0;
			}
			if (PHYSFS_setWriteDir(path) == 0) {
				PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
				int i = 0;
			}

			PHYSFS_file* myfile = PHYSFS_openWrite(file);
			if (myfile == NULL) {
				PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
				int i = 0;
			}
			int length_writed = PHYSFS_write(myfile, buff, 1, buff_size);
			PHYSFS_close(myfile);

			if (PHYSFS_unmount(path) == 0) {
				PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
				int i = 0;
			}
			if (PHYSFS_mount(".", 0, 0) == 0) {
				PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
				int i = 0;
			}
			if (PHYSFS_setWriteDir(_pref_directory.c_str()) == 0) {
				PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
				int i = 0;
			}
		}

		std::string Read(const char* filepath)
		{
			std::string ret("");
			if (PHYSFS_exists(filepath) != 0)
			{
				PHYSFS_file* myfile2 = PHYSFS_openRead(filepath);
				PHYSFS_sint64 file_size = PHYSFS_fileLength(myfile2);
				char* myBuf2 = new char[file_size];
				long long length_readed = PHYSFS_read(myfile2, myBuf2, 1, file_size);
				ret.assign(myBuf2, file_size);
				delete[] myBuf2;
				PHYSFS_close(myfile2);
				return ret;
			}
			return ret;
		}

		std::string ReadOutside(const char* filepath)
		{
			if (PHYSFS_unmount(".") == 0) {
				PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
				int i = 0;
			}
			std::string _path(filepath);
			std::string _dir(_path);
			std::string _file;
			_dir = _path.substr(0, _path.find_last_of('\\') + 1);
			_file = _path.substr(_dir.size());
			PHYSFS_mount(_dir.c_str(), "Read/", 0);

			std::string _read_path("Read/");
			_read_path += _file;
			std::string ret("");
			if (PHYSFS_exists(_read_path.c_str()) != 0)
			{
				PHYSFS_file* myfile2 = PHYSFS_openRead(_read_path.c_str());
				PHYSFS_sint64 file_size = PHYSFS_fileLength(myfile2);
				char* myBuf2 = new char[file_size];
				long long length_readed = PHYSFS_read(myfile2, myBuf2, 1, file_size);
				ret.assign(myBuf2, file_size);
				delete[] myBuf2;
				PHYSFS_close(myfile2);
				return ret;
			}

			PHYSFS_unmount(_dir.c_str());
			if (PHYSFS_mount(".", 0, 0) == 0) {
				PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
				int i = 0;
			}
			return ret;
		}

		const char* GetExecutableDirectory()
		{
			return _exec_directory.c_str();
		}

		const char* GetPrefDirectory()
		{
			return _pref_directory.c_str();
		}

		std::vector<std::string> GetFilespathFrom(const char* _path)
		{
			std::vector<std::string> ret;
			if (PHYSFS_exists(_path) != 0) {

				std::string path(_path);
				char** rc = PHYSFS_enumerateFiles(path.c_str());
				for (char** i = rc; *i != NULL; i++)
				{
					std::string inPath(path);
					inPath += *i;
					ret.push_back(inPath);
				}
			}
			return ret;
		}

		std::vector<std::string> GetFilesnameFrom(const char* _path)
		{
			std::vector<std::string> ret;

			if (PHYSFS_exists(_path) != 0) {

				std::string path(_path);
				char** rc = PHYSFS_enumerateFiles(path.c_str());
				for (char** i = rc; *i != NULL; i++)
					ret.push_back(*i);
			}
			return ret;
		}
	}
}