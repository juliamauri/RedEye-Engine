#ifndef __ASSETS_WINDOW__
#define __ASSETS_WINDOW__

class AssetsWindow : public EditorWindow
{
public:

	AssetsWindow() : EditorWindow("Assets", true) {}
	~AssetsWindow() {}

	const char* GetCurrentDirPath() const { return currentPath; }
	void SelectUndefined(eastl::string* toFill) { selectingUndefFile = toFill; }

private:

	void Draw(bool secondary = false) override;

private:

	const char* currentPath = nullptr;
	eastl::string* selectingUndefFile = nullptr;
};

#endif //!__ASSETS_WINDOW__