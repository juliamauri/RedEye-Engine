#ifndef __SKYBOX_EDITOR_WINDOW__
#define __SKYBOX_EDITOR_WINDOW__

class SkyBoxEditorWindow :public EditorWindow
{
public:

	SkyBoxEditorWindow();
	~SkyBoxEditorWindow();

private:

	void Draw(bool secondary = false) override;

private:

	eastl::string sbName, assetPath;

	class RE_SkyBox* editingSkybox = nullptr;
	unsigned int previewImage = 0;
};

#endif // !__SKYBOX_EDITOR_WINDOW__