#ifndef __MODULERENDER3D_H__
#define __MODULERENDER3D_H__

#include "Module.h"

class ShaderManager;
class Texture2DManager;
class Texture2D;
class RE_Camera;

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

	//Renderer Test Window
	ShaderType GetShaderEnabled() const;
	Texture2DType GetTexture2DEnabled() const;
	ObjectType GetObjectEnabled() const;

	void SetShaderEnabled(ShaderType shader_enabled);
	void SetTexture2DEnabled(Texture2DType texture2d_enabled);
	void SetObjectEnabled(ObjectType object_enabled);

	void UseShader(ShaderType shader_enabled);
	void SetShaderBool(const char* name, bool value);

	void ResetCamera();

	bool* GetB_EBO();
	bool* Getprintvertextcolor();
	bool* GetisRotated();
	bool* GetisScaled();
	bool* GetisCubes();
	bool* GetisMove();

	void ResetAspectRatio();

private:
	RE_Camera* camera;

	ShaderManager* shader_manager;
	unsigned int sinusColor, vertexColor, textureSquare, twotextures, shader_cube;

	Texture2DManager* texture_manager;
	unsigned int puppie1, puppie2, container, awesomeface;

	//Renderer Test
	unsigned int VAO_Triangle, VAO_Square, VAO_Cube, VBO_Triangle, VBO_Square, VBO_Cube, EBO_Square;
	bool B_EBO = true, isLine = false, vsync = false, printvertextcolor = false;

	ShaderType shaderenabled = SIN;
	Texture2DType textureEnabled = PUPPIE_1;
	ObjectType objectEnabled = PLANE;

	float timeValue = 0, timerotateValue = 0, timeCuberotateValue = 0;

	bool isRotated = false, isScaled = false, isCubes = false, isMove = false;

	float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
	float pitch = 0.0f;
};

#endif // !__MODULERENDER3D_H__