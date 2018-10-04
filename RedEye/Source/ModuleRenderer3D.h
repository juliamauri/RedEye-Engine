#ifndef __MODULERENDER3D_H__
#define __MODULERENDER3D_H__

#include "RE_Math.h"
#include "Module.h"

class ShaderManager;
class Texture2DManager;
class Texture2D;
class RE_CompCamera;
class RE_CompMesh;
class RE_Mesh;


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
	void enableDepthTest(const bool enable);
	void enableFaceCulling(const bool enable);
	void enableLighting(const bool enable);

	//Shaders - A vector in GLSL contains 4 component
	unsigned int GetMaxVertexAttributes(); //it's usually 16

	//Draws
	void DirectDrawCube(math::vec position, math::vec color);

	//Renderer Test Window
	void ResetCamera();

	void ResetAspectRatio();

	RE_CompCamera* camera;

private:
	bool vsync = false;
	bool cullface = false;
	bool depthtest = true;
	bool lighting = false;
	bool isLine = false;



	float timeValue = 0, timerotateValue = 0, timeCuberotateValue = 0, timeLight = 0;
	
	math::float2 mouse_init = math::float2::zero;
	math::float2 mouse_dest = math::float2::zero;

	bool firstMouse = true;
	float lastx = 0.0f;
	float lasty = 0.0f;
	float newx = 0.0f;
	float newy = 0.0f;
	float yaw = 0.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
	float pitch = 0.0f;
};

#endif // !__MODULERENDER3D_H__