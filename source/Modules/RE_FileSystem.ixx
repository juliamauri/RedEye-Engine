module;

#include <cstdint>
#include <physfs.h>
#include <string>
#include <vector>

export module FileSystem;

namespace
{
    std::string _exec_directory;
    std::string _pref_directory;
} // namespace

export namespace RE
{
    namespace FileSystem
    {
        /**
         * @brief Initializes the file system.
         * @param argv The command line arguments.
         * @param org The organization name.
         * @param app The application name.
         * @return True if initialization was successful, false otherwise.
         */
        bool Init(char* argv[], const char* org, const char* app)
        {
            if (PHYSFS_init(argv[0]) != 0)
            {

                _pref_directory = PHYSFS_getPrefDir(org, app);
                _exec_directory = argv[0];
#ifdef _WIN32
                _exec_directory = _exec_directory.substr(0, _exec_directory.find_last_of('\\') + 1);
#else
                _exec_directory = _exec_directory.substr(0, _exec_directory.find_last_of('/') + 1);
#endif

                if (PHYSFS_mount(".", 0, 0) == 0)
                {
                    PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
                    return false;
                }
                if (PHYSFS_mount(_pref_directory.c_str(), "WriteDir/", 0) == 0)
                {
                    PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
                    return false;
                }
                if (PHYSFS_setWriteDir(_pref_directory.c_str()) == 0)
                {
                    PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
                    return false;
                }
            }
            return true;
        }

        /**
         * @brief Cleans up the file system.
         */
        void CleanUp()
        {
            PHYSFS_deinit();
        }

        /**
         * @brief Checks if a file or directory exists.
         * @param path The path to check.
         * @return True if the file or directory exists, false otherwise.
         */
        bool Exist(const char* path)
        {
            return PHYSFS_exists(path) != 0;
        }

        /**
         * @brief Creates a new directory.
         * @param dir The directory to create.
         */
        void NewDir(const char* dir)
        {
            if (PHYSFS_exists(dir) != 0)
                return;
            if (PHYSFS_mkdir(dir) == 0)
                PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
            // else already exists
        }

        /**
         * @brief Writes data to a file.
         * @param path The path to the directory.
         * @param file The name of the file.
         * @param buff The buffer containing the data to write.
         * @param buff_size The size of the buffer.
         */
        void Write(const char* path, const char* file, const char* buff, uint32_t buff_size)
        {
            std::string filepath(std::string(path) + std::string(file));

            NewDir(path);

            PHYSFS_file* myfile = PHYSFS_openWrite(filepath.c_str());
            int length_writed = PHYSFS_write(myfile, buff, 1, buff_size);
            PHYSFS_close(myfile);
        }

        /**
         * @brief Writes data to a file outside the mounted directories.
         * @param path The path to the directory.
         * @param file The name of the file.
         * @param buff The buffer containing the data to write.
         * @param buff_size The size of the buffer.
         */
        void WriteOutside(const char* path, const char* file, const char* buff, uint32_t buff_size)
        {
            if (PHYSFS_unmount(".") == 0)
            {
                PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
                int i = 0;
            }
            if (PHYSFS_mount(path, 0, 0) == 0)
            {
                PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
                int i = 0;
            }
            if (PHYSFS_setWriteDir(path) == 0)
            {
                PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
                int i = 0;
            }

            PHYSFS_file* myfile = PHYSFS_openWrite(file);
            if (myfile == NULL)
            {
                PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
                int i = 0;
            }
            int length_writed = PHYSFS_write(myfile, buff, 1, buff_size);
            PHYSFS_close(myfile);

            if (PHYSFS_unmount(path) == 0)
            {
                PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
                int i = 0;
            }
            if (PHYSFS_mount(".", 0, 0) == 0)
            {
                PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
                int i = 0;
            }
            if (PHYSFS_setWriteDir(_pref_directory.c_str()) == 0)
            {
                PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
                int i = 0;
            }
        }

        /**
         * @brief Reads data from a file.
         * @param filepath The path to the file.
         * @return The contents of the file as a string.
         */
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

        /**
         * @brief Reads data from a file outside the mounted directories.
         * @param filepath The path to the file.
         * @return The contents of the file as a string.
         */
        std::string ReadOutside(const char* filepath)
        {
            if (PHYSFS_unmount(".") == 0)
            {
                PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
                int i = 0;
            }
            std::string _path(filepath);
            std::string _dir(_path);
            std::string _file;
#ifdef _WIN32
            _dir = _path.substr(0, _path.find_last_of('\\') + 1);
#else
            _dir = _path.substr(0, _path.find_last_of('/') + 1);
#endif
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
            if (PHYSFS_mount(".", 0, 0) == 0)
            {
                PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
                int i = 0;
            }
            return ret;
        }

        /**
         * @brief Gets the executable directory.
         * @return The executable directory as a string.
         */
        const char* GetExecutableDirectory()
        {
            return _exec_directory.c_str();
        }

        /**
         * @brief Gets the preference directory.
         * @return The preference directory as a string.
         */
        const char* GetPrefDirectory()
        {
            return _pref_directory.c_str();
        }

        /**
         * @brief Gets the file paths from a directory.
         * @param _path The path to the directory.
         * @return A vector of file paths as strings.
         */
        std::vector<std::string> GetFilespathFrom(const char* _path)
        {
            std::vector<std::string> ret;
            if (PHYSFS_exists(_path) != 0)
            {

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

        /**
         * @brief Gets the file names from a directory.
         * @param _path The path to the directory.
         * @return A vector of file names as strings.
         */
        std::vector<std::string> GetFilesnameFrom(const char* _path)
        {
            std::vector<std::string> ret;

            if (PHYSFS_exists(_path) != 0)
            {

                std::string path(_path);
                char** rc = PHYSFS_enumerateFiles(path.c_str());
                for (char** i = rc; *i != NULL; i++)
                    ret.push_back(*i);
            }
            return ret;
        }
    } // namespace FileSystem
} // namespace RE