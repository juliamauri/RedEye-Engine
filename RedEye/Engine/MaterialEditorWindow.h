#ifndef __MATERIAL_EDITOR_WINDOW__
#define __MATERIAL_EDITOR_WINDOW__

#include "EditorWindow.h"

#include <EASTL/string.h>

class MaterialEditorWindow :public EditorWindow
{
public:
	MaterialEditorWindow();
	~MaterialEditorWindow();

private:

	void Draw(bool secondary = false) override;

private:

	class RE_Material* editingMaerial = nullptr;
	eastl::string matName, assetPath;
};

#endif // !__MATERIAL_EDITOR_WINDOW__