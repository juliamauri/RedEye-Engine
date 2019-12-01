#include "RE_InternalResources.h"

#include "Application.h"

#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"

#include "RE_ShaderImporter.h"
#include "RE_TextureImporter.h"

#include "RE_SkyBox.h"
#include "RE_Shader.h"

#include "OutputLog.h"
#include "Globals.h"

#include "Glew/include/glew.h"
#include <gl/GL.h>

#include <vector>
#include <string>

#define SKYBOXDEFSIZE 2000.0f

RE_InternalResources::RE_InternalResources()
{
}

RE_InternalResources::~RE_InternalResources()
{
	if(checkerTexture != 0) glDeleteTextures(1, &checkerTexture);
	DEL(defaultShader);
	DEL(skyboxShader);
}

bool RE_InternalResources::Init()
{
	bool ret = true;

	ret = InitShaders();

	ret = InitChecker();

	ret = InitSkyBox();

	return false;
}

bool RE_InternalResources::InitShaders()
{
	bool ret = true;

	//Loading Shaders
	if (App->shaders)
	{
		defaultShader = new RE_Shader();
		defaultShader->SetName("Default Shader");
		defaultShader->SetType(Resource_Type::R_SHADER);
		defaultShader->SetInternal(true);
		defaultShader->SetPaths("Library/Shaders/default.vert", "Library/Shaders/default.frag");
		defaultShader->LoadInMemory();

		skyboxShader = new RE_Shader();
		skyboxShader->SetName("SkyBox Shader");
		skyboxShader->SetType(Resource_Type::R_SHADER);
		skyboxShader->SetInternal(true);
		skyboxShader->SetPaths("Library/Shaders/skybox.vert", "Library/Shaders/skybox.frag");
		skyboxShader->LoadInMemory();
	}

	return ret;
}

bool RE_InternalResources::InitChecker()
{
	// Checkers
	int value;
	int IMAGE_ROWS = 264;
	int IMAGE_COLS = 264;
	GLubyte imageData[264][264][3];
	for (int row = 0; row < IMAGE_ROWS; row++) {
		for (int col = 0; col < IMAGE_COLS; col++) {
			// Each cell is 8x8, value is 0 or 255 (black or white)
			value = (((row & 0x8) == 0) ^ ((col & 0x8) == 0)) * 255;
			imageData[row][col][0] = (GLubyte)value;
			imageData[row][col][1] = (GLubyte)value;
			imageData[row][col][2] = (GLubyte)value;
		}
	}
	
	glGenTextures(1, &checkerTexture);
	glBindTexture(GL_TEXTURE_2D, checkerTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, IMAGE_COLS, IMAGE_ROWS, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

bool RE_InternalResources::InitSkyBox()
{
	const char* rightTex = App->resources->ImportTexture("Assets/Skyboxes/default/1right.jpg");
	const char* leftTex = App->resources->ImportTexture("Assets/Skyboxes/default/2left.jpg");
	const char* topTex = App->resources->ImportTexture("Assets/Skyboxes/default/3top.jpg");
	const char* bottomTex = App->resources->ImportTexture("Assets/Skyboxes/default/4bottom.jpg");
	const char* frontTex = App->resources->ImportTexture("Assets/Skyboxes/default/5front.jpg");
	const char* backTex = App->resources->ImportTexture("Assets/Skyboxes/default/6back.jpg");

	defaultSkybox = new RE_SkyBox();
	defaultSkybox->SetName("defaultSkyBox");
	defaultSkybox->SetType(Resource_Type::R_SKYBOX);
	defaultSkybox->AddTexture(RE_TextureFace::RE_RIGHT, rightTex);
	defaultSkybox->AddTexture(RE_TextureFace::RE_LEFT, leftTex);
	defaultSkybox->AddTexture(RE_TextureFace::RE_TOP, topTex);
	defaultSkybox->AddTexture(RE_TextureFace::RE_BOTTOM, bottomTex);
	defaultSkybox->AddTexture(RE_TextureFace::RE_FRONT, frontTex);
	defaultSkybox->AddTexture(RE_TextureFace::RE_BACK, backTex);
	defaultSkybox->AssetSave();
	defaultSkybox->SaveMeta();
	App->resources->Reference(defaultSkybox);
	App->resources->Use(defaultSkybox->GetMD5());
	return true;
}

unsigned int RE_InternalResources::GetDefaultShader() const
{
	return defaultShader->GetID();
}

unsigned int RE_InternalResources::GetSkyBoxShader() const
{
	return skyboxShader->GetID();
}

unsigned int RE_InternalResources::GetTextureChecker() const
{
	return checkerTexture;
}

unsigned int RE_InternalResources::GetSkyBoxVAO() const
{
	return defaultSkybox->GetVAO();
}

unsigned int RE_InternalResources::GetSkyBoxTexturesID() const
{
	return defaultSkybox->GetID();
}
