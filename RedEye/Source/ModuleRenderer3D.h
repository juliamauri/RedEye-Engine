#ifndef __MODULERENDER3D_H__
#define __MODULERENDER3D_H__

#include "Module.h"

class Shader;
class Texture2DManager;

class ModuleRenderer3D : public Module 
{

public:
	ModuleRenderer3D(const char* name, bool start_enabled = true);
	~ModuleRenderer3D();

	bool Init(JSONNode* config_module) override;
	update_status PreUpdate() override;
	update_status PostUpdate() override;
	bool CleanUp() override;

	void* mainContext;
	unsigned int renderedTexture;

	//Shaders - A vector in GLSL contains 4 component
	unsigned int GetMaxVertexAttributes(); //it's usually 16

private:
	unsigned int VAO_Triangle, VAO_Square, VBO_Triangle, VBO_Square, EBO_Square;
	bool B_EBO = false;
	bool isLine = false;

	float timeValue = 0;
	Shader* sinusColor;
	Shader* vertexColor;
	bool v_color = true;

	Texture2DManager* texture_manager;
};

#endif // !__MODULERENDER3D_H__