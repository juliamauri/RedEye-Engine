#ifndef __TEXT_EDITOR_MANAGER_WINDOW__
#define __TEXT_EDITOR_MANAGER_WINDOW__

#include "EditorWindow.h"
#include <EASTL/string.h>
#include <EASTL/vector.h>

class TextEditorManagerWindow :public EditorWindow
{
public:
	TextEditorManagerWindow() : EditorWindow("Text Editor Manager", false) {}
	~TextEditorManagerWindow();

	void PushEditor(const char* filePath, eastl::string* newFile = nullptr, const char* shadertTemplate = nullptr, bool* open = nullptr);

private:
	void Draw(bool secondary = false) override;

	struct EditorData
	{
		eastl::string* toModify = nullptr;
		class TextEditor* textEditor = nullptr;
		class RE_FileBuffer* file = nullptr;
		bool save = false;
		bool* open = nullptr;
		bool compiled = false;
		bool works = false;
	};

	eastl::vector<EditorData*> editors;
};

#endif // !__TEXT_EDITOR_MANAGER_WINDOW__