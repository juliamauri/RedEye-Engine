module;

#include <codecvt>
#include <locale>
#include <nfd.h>
#include <string>
#include <vector>

export module Dialogs;

namespace
{
    /**
     * @brief Converts a wide character string to a UTF-8 encoded string.
     * @param wstr The wide character string to convert.
     * @return The UTF-8 encoded string.
     */
    std::string WCharToString(const wchar_t* wstr)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.to_bytes(wstr);
    }

    /**
     * @brief Converts a UTF-8 encoded string to a wide character string.
     * @param str The UTF-8 encoded string to convert.
     * @return The wide character string.
     */
    std::wstring CharToWChar(const char* str)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.from_bytes(str);
    }
} // namespace

export namespace RE
{
    namespace Dialogs
    {
        /**
         * @brief Initializes the native file dialog library.
         * @return True if initialization was successful, false otherwise.
         */
        bool Init()
        {
            return NFD_Init() == NFD_OKAY;
        }

        /**
         * @brief Shuts down the native file dialog library.
         */
        void Quit()
        {
            NFD_Quit();
        }

        /**
         * @brief Opens a file dialog with the specified filter and default
         * path.
         * @param filterList The filter list for the file dialog.
         * @param defaultPath The default path for the file dialog.
         * @return The selected file path as a string.
         */
        std::string OpenDialog(const char* filterList = nullptr, const char* defaultPath = nullptr)
        {
#ifdef _WIN32
            std::wstring wDefaultPath = CharToWChar(defaultPath);
            const wchar_t* cwDefaultPath = wDefaultPath.c_str();
            std::wstring wFilterList = CharToWChar(filterList);
            const wchar_t* cwFilterList = wFilterList.c_str();
#else
            const char* cwDefaultPath = defaultPath;
            const char* cwFilterList = filterList;
#endif

            nfdnchar_t* outPath = nullptr;
            nfdnfilteritem_t filterItem = {cwFilterList, cwFilterList};
            nfdopendialognargs_t args = {&filterItem, 1, cwDefaultPath, {0, nullptr}};

            if (NFD_OpenDialogN_With(&outPath, &args) != NFD_OKAY)
            {
                return "";
            }
#ifdef _WIN32
            std::string path(WCharToString(outPath));
#else
            std::string path(outPath);
#endif
            NFD_FreePathN(outPath);
            return path;
        }

        /**
         * @brief Opens a file dialog with the specified filter and default
         * path, allowing multiple selections.
         * @param filterList The filter list for the file dialog.
         * @param defaultPath The default path for the file dialog.
         * @return A vector of selected file paths as strings.
         */
        std::vector<std::string> OpenDialogMultiple(const char* filterList = nullptr,
                                                    const char* defaultPath = nullptr)
        {
#ifdef _WIN32
            std::wstring wDefaultPath = CharToWChar(defaultPath);
            const wchar_t* cwDefaultPath = wDefaultPath.c_str();
            std::wstring wFilterList = CharToWChar(filterList);
            const wchar_t* cwFilterList = wFilterList.c_str();
#else
            const char* cwDefaultPath = defaultPath;
            const char* cwFilterList = filterList;
#endif

            const nfdpathset_t* outPaths = nullptr;
            nfdnfilteritem_t filterItem = {cwFilterList, cwFilterList};
            nfdopendialognargs_t args = {&filterItem, 1, cwDefaultPath, {0, nullptr}};

            if (NFD_OpenDialogMultipleN_With(&outPaths, &args) != NFD_OKAY)
            {
                return {};
            }

            std::vector<std::string> paths;
            nfdpathsetenum_t enumerator;
            if (NFD_PathSet_GetEnum(outPaths, &enumerator) == NFD_OKAY)
            {
                nfdnchar_t* outPath = nullptr;
                while (NFD_PathSet_EnumNextN(&enumerator, &outPath) == NFD_OKAY)
                {
#ifdef _WIN32
                    paths.push_back(WCharToString(outPath));
#else
                    paths.push_back(outPath);
#endif
                    NFD_PathSet_FreePathN(outPath);
                }
                NFD_PathSet_FreeEnum(&enumerator);
            }
            NFD_PathSet_Free(outPaths);
            return paths;
        }

        /**
         * @brief Opens a save file dialog with the specified filter and default
         * path.
         * @param filterList The filter list for the save file dialog.
         * @param defaultPath The default path for the save file dialog.
         * @return The selected file path as a string.
         */
        std::string SaveDialog(const char* filterList = nullptr, const char* defaultPath = nullptr)
        {
#ifdef _WIN32
            std::wstring wDefaultPath = CharToWChar(defaultPath);
            const wchar_t* cwDefaultPath = wDefaultPath.c_str();
            std::wstring wFilterList = CharToWChar(filterList);
            const wchar_t* cwFilterList = wFilterList.c_str();
#else
            const char* cwDefaultPath = defaultPath;
            const char* cwFilterList = filterList;
#endif

            nfdnchar_t* outPath = nullptr;
            nfdnfilteritem_t filterItem = {cwFilterList, cwFilterList};
            nfdsavedialognargs_t args = {&filterItem, 1, cwDefaultPath, L"", {0, nullptr}};

            if (NFD_SaveDialogN_With(&outPath, &args) != NFD_OKAY)
            {
                return "";
            }
#ifdef _WIN32
            std::string path(WCharToString(outPath));
#else
            std::string path(outPath);
#endif
            NFD_FreePathN(outPath);
            return path;
        }

        /**
         * @brief Opens a folder picker dialog with the specified default path.
         * @param defaultPath The default path for the folder picker dialog.
         * @return The selected folder path as a string.
         */
        std::string PickFolder(const char* defaultPath)
        {
#ifdef _WIN32
            std::wstring wDefaultPath = CharToWChar(defaultPath);
            const wchar_t* cwDefaultPath = wDefaultPath.c_str();
#else
            const char* cwDefaultPath = defaultPath;
#endif
            nfdnchar_t* outPath = nullptr;

            if (NFD_PickFolderN(&outPath, cwDefaultPath) != NFD_OKAY)
            {
                return "";
            }
#ifdef _WIN32
            std::string path(WCharToString(outPath));
#else
            std::string path(outPath);
#endif
            NFD_FreePathN(outPath);
            return path;
        }

        /**
         * @brief Opens a folder picker dialog with the specified default path,
         * allowing multiple selections.
         * @param defaultPath The default path for the folder picker dialog.
         * @return A vector of selected folder paths as strings.
         */
        std::vector<std::string> PickFolderMultiple(const char* defaultPath = nullptr)
        {
#ifdef _WIN32
            std::wstring wDefaultPath = CharToWChar(defaultPath);
            const wchar_t* cwDefaultPath = wDefaultPath.c_str();
#else
            const char* cwDefaultPath = defaultPath;
#endif
            const nfdpathset_t* outPaths = nullptr;

            if (NFD_PickFolderMultipleN(&outPaths, cwDefaultPath) != NFD_OKAY)
            {
                return {};
            }

            std::vector<std::string> paths;
            nfdpathsetenum_t enumerator;
            if (NFD_PathSet_GetEnum(outPaths, &enumerator) == NFD_OKAY)
            {
                nfdnchar_t* outPath = nullptr;
                while (NFD_PathSet_EnumNextN(&enumerator, &outPath) == NFD_OKAY)
                {
#ifdef _WIN32
                    paths.push_back(WCharToString(outPath));
#else
                    paths.push_back(outPath);
#endif
                    NFD_PathSet_FreePathN(outPath);
                }
                NFD_PathSet_FreeEnum(&enumerator);
            }
            NFD_PathSet_Free(outPaths);
            return paths;
        }
    } // namespace Dialogs
} // namespace RE