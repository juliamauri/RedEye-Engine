#ifndef __MODULERENDER3D_H__
#define __MODULERENDER3D_H__

#include "Module.h"

class Shader;
class Texture2DManager;
class Texture2D;

enum ShaderType
{
	SIN,
	VERTEX,
	TEXTURE
};

enum Texture2DType
{
	PUPPIE_1,
	PUPPIE_2,
	CONTAINER,
	MIX_AWESOMEFACE
};

class ModuleRenderer3D : public Module 
{

public:
	ModuleRenderer3D(const char* name, bool start_enabled = true);
	~ModuleRenderer3D();

	bool Init(JSONNode* config_module) override;
	update_status PreUpdate() override;
	update_status PostUpdate() override;
	bool CleanUp() override;

	void DrawEditor();
	void RecieveEvent(const Event* e) override;

	void* mainContext;
	unsigned int renderedTexture;

	void enableVSync(bool enable);

	//Shaders - A vector in GLSL contains 4 component
	unsigned int GetMaxVertexAttributes(); //it's usually 16

private:
	unsigned int VAO_Triangle, VAO_Square, VBO_Triangle, VBO_Square, EBO_Square;
	bool B_EBO = true;
	bool isLine = false;
	bool vsync = false;
	int shader_selcted = 0;
	int texture_selected = 0;

	float timeValue = 0;
	float timerotateValue = 0;
	Shader* sinusColor;
	Shader* vertexColor;
	Shader* textureSquare;
	Shader* twotextures;
	ShaderType shaderenabled = SIN;
	Texture2DType textureEnabled = PUPPIE_1;
	bool printvertextcolor = false;

	Texture2DManager* texture_manager;

	Texture2D* puppie1;
	Texture2D* puppie2;
	Texture2D* container;
	Texture2D* awesomeface;

	bool isScaled = false;
};

#endif // !__MODULERENDER3D_H__