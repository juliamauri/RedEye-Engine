#ifndef __MODULERENDER3D_H__
#define __MODULERENDER3D_H__

#include "Module.h"

class ShaderManager;
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

enum ObjectType
{
	PLANE,
	CUBE
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

	void enableVSync(const bool enable);

	//Shaders - A vector in GLSL contains 4 component
	unsigned int GetMaxVertexAttributes(); //it's usually 16

private:
	ShaderManager* shader_manager;
	unsigned int sinusColor, vertexColor, textureSquare, twotextures, shader_cube;

	ShaderType shaderenabled = SIN;
	Texture2DType textureEnabled = PUPPIE_1;
	ObjectType objectEnabled = PLANE;
	int shader_selcted = 0;
	int texture_selected = 0;
	int object_selected = 0;

	Texture2DManager* texture_manager;
	unsigned int puppie1, puppie2, container, awesomeface;

	unsigned int VAO_Triangle, VAO_Square, VAO_Cube, VBO_Triangle, VBO_Square, VBO_Cube, EBO_Square;
	bool B_EBO = true;
	bool isLine = false;
	bool vsync = false;
	bool printvertextcolor = false;


	float timeValue = 0;
	float timerotateValue = 0;
	float timeCuberotateValue = 0;

	bool isRotated = false;
	bool isScaled = false;
	bool isCubes = false;
};

#endif // !__MODULERENDER3D_H__