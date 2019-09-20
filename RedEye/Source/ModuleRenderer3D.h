#ifndef __MODULERENDER3D_H__
#define __MODULERENDER3D_H__

#include "RE_Math.h"
#include "Module.h"

class ShaderManager;
class Texture2DManager;
class Texture2D;
class RE_Camera;
class RE_CompMesh;
class RE_Mesh;


class ModuleRenderer3D : public Module 
{

public:
	ModuleRenderer3D(const char* name, bool start_enabled = true);
	~ModuleRenderer3D();

	bool Init(JSONNode* node) override;
	update_status PreUpdate() override;
	update_status PostUpdate() override;
	bool CleanUp() override;

	void DrawEditor();

	bool Load(JSONNode* node) override;
	bool Save(JSONNode* node) const override;

	void SetVSync(const bool enable);
	void SetDepthTest(const bool enable);
	void SetFaceCulling(const bool enable);
	void SetLighting(const bool enable);
	void SetTexture2D(const bool enable);
	void SetColorMaterial(const bool enable);
	void SetWireframe(const bool enable);

	//Shaders - A vector in GLSL contains 4 component
	unsigned int GetMaxVertexAttributes(); //it's usually 16

	//Draws
	void DirectDrawCube(math::vec position, math::vec color);

	//Renderer Test Window
	void ResetAspectRatio();

	//context getter
	void* GetContext()const;

private:

	void* mainContext;
	unsigned int renderedTexture;

	bool vsync = false;
	bool cullface = false;
	bool depthtest = true;
	bool lighting = false;
	bool texture2d = false;
	bool color_material = false;
	bool wireframe = false;

	//float timeValue = 0, timerotateValue = 0, timeCuberotateValue = 0, timeLight = 0;
};

#endif // !__MODULERENDER3D_H__