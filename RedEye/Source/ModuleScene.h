#ifndef __MODULESCENE_H__
#define __MODULESCENE_H__

#include "Module.h"
#include "Event.h"

class RE_Mesh;
class RE_GameObject;
class RE_CompPrimitive;
class RE_CompMesh;
class RE_CompUnregisteredMesh;

class ModuleScene : public Module
{
public:
	ModuleScene(const char* name, bool start_enabled = true);
	~ModuleScene();

	bool Start() override;

	update_status PreUpdate() override;
	update_status Update() override;
	update_status PostUpdate() override;

	bool CleanUp() override;

	void FileDrop(const char* file);

	void RecieveEvent(const Event* e) override;

	void DrawScene();

private:
	//shaders
	unsigned int sinusColor, vertexColor, textureSquare, twotextures, shader_cube, lightingShader, lampShader, lightingmapShader, modelloading;
	unsigned int ShaderPrimitive;

	//Textures
	unsigned int puppie1, puppie2, container, awesomeface, container2, container2_specular;

	//Meshes
	RE_Mesh* triangle = nullptr;
	RE_Mesh* square = nullptr;
	RE_Mesh* cube_array = nullptr;
	RE_Mesh* cube_index = nullptr;
	RE_CompUnregisteredMesh* mesh_droped = nullptr;

	RE_CompPrimitive* compcube =  nullptr;
	RE_CompPrimitive* comppoint = nullptr;
	RE_CompPrimitive* compline = nullptr;
	RE_CompPrimitive* comptriangle = nullptr;

	RE_GameObject* root = nullptr;
	RE_GameObject* drop = nullptr;
};


#endif // !__MODULESCENE_H__