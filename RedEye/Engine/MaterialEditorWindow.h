#ifndef __MATERIAL_EDITOR_WINDOW__
#define __MATERIAL_EDITOR_WINDOW__

class MaterialEditorWindow :public EditorWindow
{
public:
	MaterialEditorWindow();
	~MaterialEditorWindow() final;

private:

	void Draw(bool secondary = false) final;

private:

	class RE_Material* editingMaerial = nullptr;
	eastl::string matName, assetPath;
};

#endif // !__MATERIAL_EDITOR_WINDOW__