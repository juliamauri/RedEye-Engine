#ifndef __MODULERENDER3D_H__
#define __MODULERENDER3D_H__

#include "RE_Math.h"
#include "Module.h"

class RE_CompCamera;

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

	void RecieveEvent(const Event& e) override;
	void DrawEditor() override;

	bool Load(JSONNode* node) override;
	bool Save(JSONNode* node) const override;

	void DrawScene(const math::Frustum& frustum, unsigned int fbo, RE_CompCamera* mainSkybox = nullptr, bool debugDraw = false, bool stencilToSelected = false);

	// Editor Values
	void SetVSync(bool enable);
	void SetDepthTest(bool enable);
	void SetFaceCulling(bool enable);
	void SetLighting(bool enable);
	void SetTexture2D(bool enable);
	void SetColorMaterial(bool enable);
	void SetWireframe(bool enable);

	// Shaders - A vector in GLSL contains 4 component
	unsigned int GetMaxVertexAttributes(); //it's usually 16

	// Draws
	void DirectDrawCube(math::vec position, math::vec color);

	// Context & Viewport
	void* GetWindowContext()const;

	void ChangeFBOSize(int width, int height, bool editor = false);

	unsigned int GetRenderedEditorSceneTexture()const;
	unsigned int GetRenderedGameSceneTexture()const;

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

	// FBOs
	unsigned int sceneEditorFBO = 0;
	unsigned int sceneGameFBO = 0;
};

#endif // !__MODULERENDER3D_H__