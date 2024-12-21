#ifndef __ASSETS_WINDOW__
#define __ASSETS_WINDOW__

#include "RE_FileSystem.h"

class AssetsWindow : public EditorWindow
{
public:

	AssetsWindow() : EditorWindow("Assets", true) {}
	~AssetsWindow() final = default;

	const char* GetCurrentDirPath() const { return currentPath; }
	void SelectUndefined(eastl::string* toFill) { selectingUndefFile = toFill; }

private:

	void Draw(bool secondary = false) final;

	void DrawDirectoryItem(eastl::stack<RE_FileSystem::RE_Path*>& filesToDisplay, const eastl::string& idName, unsigned int& idCount, float iconsSize, RE_FileSystem::RE_Directory*& toChange, bool secondary);
	
	void DrawItemResource(const RE_FileSystem::RE_Path* p, float iconsSize, eastl::string& id, const eastl::string& idName, const unsigned int& idCount) const;
	void DrawItemNotSupported(bool secondary, float iconsSize, const RE_FileSystem::RE_Path* p, eastl::string& id, const eastl::string& idName, const unsigned int& idCount);
	void DrawItemMeta(const RE_FileSystem::RE_Path* p, float iconsSize, eastl::string& id, const eastl::string& idName, const unsigned int& idCount) const;
	void DrawItemFolder(float iconsSize, RE_FileSystem::RE_Directory*& toChange, const RE_FileSystem::RE_Path* p) const;

	void DrawPopUpDeleteResource(eastl::string& id, const eastl::string& idName, const unsigned int& idCount, const ResourceContainer* res) const;

	void DrawDisplayOptions(float& iconsSize);
	void DrawDirectories(RE_FileSystem::RE_Directory* currentDir, RE_FileSystem::RE_Directory*& toChange);


	const char* currentPath = nullptr;
	eastl::string* selectingUndefFile = nullptr;
};

#endif //!__ASSETS_WINDOW__