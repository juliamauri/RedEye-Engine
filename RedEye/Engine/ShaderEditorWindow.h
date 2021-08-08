#ifndef __SHADER_EDITOR_WINDOW__
#define __SHADER_EDITOR_WINDOW__

#include "EditorWindow.h"
#include <EASTL/string.h>

class ShaderEditorWindow :public EditorWindow
{
public:

	ShaderEditorWindow();
	~ShaderEditorWindow();

private:

	void Draw(bool secondary = false) override;

private:

	class RE_Shader* editingShader = nullptr;

	eastl::string
		shaderName, assetPath,
		vertexPath, fragmentPath, geometryPath;
};

#endif // !__SHADER_EDITOR_WINDOW__