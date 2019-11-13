#ifndef __MODULERENDER3D_H__
#define __MODULERENDER3D_H__

#include "RE_Math.h"
#include "Module.h"
#include <list>

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

	bool Init(JSONNode* node) override;
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

	// Camera
	RE_CompCamera * CurrentCamera() const;
	void ResetAspectRatio(float width, float height);
	bool HasMainCamera() const;
	void AddMainCamera(RE_CompCamera* cam);
	const std::list<RE_CompCamera*>& GetCameras() const;
	void ResetSceneCameras();

	// Shaders - A vector in GLSL contains 4 component
	unsigned int GetMaxVertexAttributes(); //it's usually 16

	// Draws
	void DirectDrawCube(math::vec position, math::vec color);

	// Context
	void* GetWindowContext()const;

private:

	// Context
	void* mainContext;
	
	// Cameras
	RE_CompCamera* editor_camera = nullptr;
	RE_CompCamera* main_camera = nullptr;
	std::list<RE_CompCamera*> scene_cameras;
	bool cull_scene = true;

	// Configuration
	bool vsync = false;
	bool cullface = false;
	bool depthtest = true;
	bool lighting = false;
	bool texture2d = false;
	bool color_material = false;
	bool wireframe = false;
};

#endif // !__MODULERENDER3D_H__