#ifndef __MODULERENDER3D_H__
#define __MODULERENDER3D_H__

#include "RE_Math.h"
#include "Module.h"
#include <list>

class ModuleRenderer3D : public Module 
{

public:
	ModuleRenderer3D(const char* name, bool start_enabled = true);
	~ModuleRenderer3D();

	bool Init(JSONNode* node) override;
	bool Start() override;
	update_status PreUpdate() override;
	update_status PostUpdate() override;
	bool CleanUp() override;

	void DrawEditor();

	bool Load(JSONNode* node) override;
	bool Save(JSONNode* node) const override;

	// Editor Values
	void SetVSync(bool enable);
	void SetDepthTest(bool enable);
	void SetFaceCulling(bool enable);
	void SetLighting(bool enable);
	void SetTexture2D(bool enable);
	void SetColorMaterial(bool enable);
	void SetWireframe(bool enable);
	bool GetLighting() const;

	// Shaders - A vector in GLSL contains 4 component
	unsigned int GetMaxVertexAttributes(); //it's usually 16

	// Draws
	void DirectDrawCube(math::vec position, math::vec color);

	// Context
	void* GetWindowContext()const;
	void WindowSizeChanged(int width, int height);

	uint GetShaderScene() const;

private:

	// Context
	void* mainContext;

	// Configuration
	bool vsync = false;
	bool cullface = false;
	bool depthtest = true;
	bool lighting = false;
	bool texture2d = false;
	bool color_material = false;
	bool wireframe = false;
	bool cull_scene = true;

	// Default Shaders
	unsigned int sceneShader = 0;
	unsigned int skyboxShader = 0;
};

#endif // !__MODULERENDER3D_H__