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

enum Texzture2DType
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

	void RecieveEvent(const Event* e) override;

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
	Shader* textureSquare;
	Shader* twotextures;
	ShaderType shaderenabled = SIN;
	Texzture2DType textureEnabled = PUPPIE_1;
	bool printvertextcolor = false;

	Texture2DManager* texture_manager;

	Texture2D* puppie1;
	Texture2D* puppie2;
	Texture2D* container;
	Texture2D* awesomeface;
};

#endif // !__MODULERENDER3D_H__