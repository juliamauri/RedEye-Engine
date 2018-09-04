#ifndef __MODULERENDER3D_H__
#define __MODULERENDER3D_H__

#include "Module.h"


class ModuleRenderer3D : public Module 
{

public:
	ModuleRenderer3D(const char* name, bool start_enabled = true);
	~ModuleRenderer3D();

	bool Init(JSONNode* config_module) override;
	update_status PreUpdate() override;
	update_status PostUpdate() override;
	bool CleanUp() override;

	void* mainContext;
	unsigned int renderedTexture;

private:
	unsigned int shaderProgram;
	unsigned int EBO;
	unsigned int VAO;
	bool B_EBO = true;

	void LoadBuffer(const char* path, char** buffer, unsigned int size);
};

#endif // !__MODULERENDER3D_H__