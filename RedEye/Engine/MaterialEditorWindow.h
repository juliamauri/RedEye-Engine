#ifndef __MATERIAL_EDITOR_WINDOW__
#define __MATERIAL_EDITOR_WINDOW__

#include "EditorWindow.h"
#include <EASTL/string.h>

class RE_Material;

class MaterialEditorWindow :public EditorWindow
{
public:
	MaterialEditorWindow();
	~MaterialEditorWindow() final;

private:

	void Draw(bool secondary = false) final;

private:

	RE_Material* editing_material = nullptr;
	eastl::string matName;
	eastl::string assetPath;
};

#endif // !__MATERIAL_EDITOR_WINDOW__