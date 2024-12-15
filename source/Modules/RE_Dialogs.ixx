module;

#include <nfd.h>

#include <string>
#include <vector>
#include <codecvt>

export module Dialogs;

namespace {
	std::string WCharToString(const wchar_t* wstr)
	{
		// Create a wstring_convert object using codecvt_utf8
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

		// Convert the wchar_t* to std::string
		std::string str = converter.to_bytes(wstr);

		return str;
	}

	wchar_t* CharToWChar(const char* str)
	{
		if (str == nullptr) return nullptr;
		size_t len = std::strlen(str) + 1;
		wchar_t* wstr = new wchar_t[len];
		std::mbstowcs(wstr, str, len);
		return wstr;
	}
}

export namespace RE {
	namespace Dialogs {
		bool Init()
		{
			return NFD_Init() == NFD_OKAY;
		}

		void Quit()
		{
			NFD_Quit();
		}

		std::string OpenDialog(const wchar_t* filterList, const char* defaultPath)
		{
			nfdnchar_t* outPath = nullptr;
			nfdnfilteritem_t filterItem = { filterList, filterList };
			wchar_t* wDefaultPath = CharToWChar(defaultPath);
			nfdopendialognargs_t args = { &filterItem, 1, wDefaultPath, { 0, nullptr } };
			if(wDefaultPath) delete[] wDefaultPath;
			if (NFD_OpenDialogN_With(&outPath, &args) == NFD_OKAY)
			{
				std::string path(WCharToString(outPath));
				NFD_FreePathN(outPath);
				return path;
			}
			return "";
		}

		std::vector<std::string> OpenDialogMultiple(const wchar_t* filterList, const char* defaultPath)
		{
			const nfdpathset_t* outPaths = nullptr;
			nfdnfilteritem_t filterItem = { filterList, nullptr };
			wchar_t* wDefaultPath = CharToWChar(defaultPath);
			nfdopendialognargs_t args = { &filterItem, 1, wDefaultPath, { 0, nullptr } };
			if (wDefaultPath) delete[] wDefaultPath;
			if (NFD_OpenDialogMultipleN_With(&outPaths, &args) == NFD_OKAY)
			{
				std::vector<std::string> paths;
				nfdpathsetenum_t enumerator;
				if (NFD_PathSet_GetEnum(outPaths, &enumerator) == NFD_OKAY)
				{
					nfdnchar_t* outPath = nullptr;
					while (NFD_PathSet_EnumNextN(&enumerator, &outPath) == NFD_OKAY)
					{
						paths.push_back(WCharToString(outPath));
						NFD_PathSet_FreePathN(outPath);
					}
					NFD_PathSet_FreeEnum(&enumerator);
				}
				NFD_PathSet_Free(outPaths);
				return paths;
			}
			return {};
		}

		std::string SaveDialog(const wchar_t* filterList, const char* defaultPath)
		{
			nfdnchar_t* outPath = nullptr;
			nfdnfilteritem_t filterItem = { filterList, nullptr };
			wchar_t* wDefaultPath = CharToWChar(defaultPath);
			nfdsavedialognargs_t args = { &filterItem, 1, wDefaultPath, L"", { 0, nullptr } };
			if (wDefaultPath) delete[] wDefaultPath;
			if (NFD_SaveDialogN_With(&outPath, &args) == NFD_OKAY)
			{
				std::string path(WCharToString(outPath));
				NFD_FreePathN(outPath);
				return path;
			}
			return "";
		}

		std::string PickFolder(const char* defaultPath)
		{
			nfdnchar_t* outPath = nullptr;
			wchar_t* wDefaultPath = CharToWChar(defaultPath);
			if (NFD_PickFolderN(&outPath, wDefaultPath) == NFD_OKAY)
			{
				if (wDefaultPath) delete[] wDefaultPath;
				std::string path(WCharToString(outPath));
				NFD_FreePathN(outPath);
				return path;
			}
			if (wDefaultPath) delete[] wDefaultPath;
			return "";
		}

		std::vector<std::string> PickFolderMultiple(const char* defaultPath)
		{
			const nfdpathset_t* outPaths = nullptr;
			wchar_t* wDefaultPath = CharToWChar(defaultPath);
			if (NFD_PickFolderMultipleN(&outPaths, wDefaultPath) == NFD_OKAY)
			{
				if (wDefaultPath) delete[] wDefaultPath;
				std::vector<std::string> paths;
				nfdpathsetenum_t enumerator;
				if (NFD_PathSet_GetEnum(outPaths, &enumerator) == NFD_OKAY)
				{
					nfdnchar_t* outPath = nullptr;
					while (NFD_PathSet_EnumNextN(&enumerator, &outPath) == NFD_OKAY)
					{
						paths.push_back(WCharToString(outPath));
						NFD_PathSet_FreePathN(outPath);
					}
					NFD_PathSet_FreeEnum(&enumerator);
				}
				NFD_PathSet_Free(outPaths);
				return paths;
			}
			if (wDefaultPath) delete[] wDefaultPath;
			return {};
		}
	}
}