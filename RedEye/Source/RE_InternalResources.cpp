#include "RE_InternalResources.h"

#include "Application.h"

#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"
#include "ModuleRenderer3D.h"

#include "RE_DefaultShaders.h"
#include "RE_ShaderImporter.h"
#include "RE_TextureImporter.h"

#include "RE_SkyBox.h"
#include "RE_Shader.h"
#include "RE_Material.h"

#include "OutputLog.h"
#include "Globals.h"
#include "RE_GLCacheManager.h"

#include "Glew/include/glew.h"
#include <gl/GL.h>

RE_InternalResources::RE_InternalResources() {}

RE_InternalResources::~RE_InternalResources()
{
	if(checkerTexture != 0) glDeleteTextures(1, &checkerTexture);
}

void RE_InternalResources::Init()
{
	InitChecker();
	if (!InitShaders()) RE_LOG_WARNING("Could not initialize default shaders");
	if (!InitMaterial()) RE_LOG_WARNING("Could not initialize default materials");
	if (!InitSkyBox()) RE_LOG_WARNING("Could not initialize default skybox");
}

void RE_InternalResources::InitChecker()
{
	// Checkers
	int IMAGE_ROWS = 264, IMAGE_COLS = 264;
	GLubyte imageData[264][264][3];
	for (int row = 0; row < IMAGE_ROWS; row++)
	{
		for (int col = 0; col < IMAGE_COLS; col++)
		{
			GLubyte value = (((row & 0x8) == 0) ^ ((col & 0x8) == 0)) * 255; // Each cell is 8x8, value is 0 or 255 (black or white)
			imageData[row][col][0] = value;
			imageData[row][col][1] = value;
			imageData[row][col][2] = value;
		}
	}

	glGenTextures(1, &checkerTexture);
	RE_GLCacheManager::ChangeTextureBind(checkerTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, IMAGE_COLS, IMAGE_ROWS, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}

bool RE_InternalResources::InitShaders()
{
	//Loading Shaders
	if (App::resources)
	{
		// Default
		RE_Shader* defSRes = new RE_Shader();
		defSRes->SetName("Default Shader");
		defSRes->SetType(Resource_Type::R_SHADER);
		defSRes->SetAsInternal(DEFVERTEXSHADER, DEFFRAGMENTSHADER);
		defaultShader = App::resources->Reference(defSRes);

		// Scaled (for outline)
		RE_Shader* defScaleRes = new RE_Shader();
		defScaleRes->SetName("Default Scale Shader");
		defScaleRes->SetType(Resource_Type::R_SHADER);
		defScaleRes->SetAsInternal(DEFVERTEXSCALESHADER, DEFFRAGMENTSHADER);
		defaultScaleShader = App::resources->Reference(defScaleRes);

		// Skybox
		RE_Shader* defSKRes = new RE_Shader();
		defSKRes->SetName("Default SkyBox Shader");
		defSKRes->SetType(Resource_Type::R_SHADER);
		defSKRes->SetAsInternal(SKYBOXVERTEXSHADER, SKYBOXFRAGMENTSHADER);
		skyboxShader = App::resources->Reference(defSKRes);

		// Deferred
		RE_Shader* deferred = new RE_Shader();
		deferred->SetName("Deferred Shader");
		deferred->SetType(Resource_Type::R_SHADER);
		deferred->SetAsInternal(GEOPASSVERTEXSHADER, GEOPASSFRAGMENTSHADER);
		defGeoShader = App::resources->Reference(deferred);

		// Light Pass
		RE_Shader* lightPass = new RE_Shader();
		lightPass->SetName("Light Pass Shader");
		lightPass->SetType(Resource_Type::R_SHADER);
		lightPass->SetAsInternal(LIGHTPASSVERTEXSHADER, LIGHTPASSFRAGMENTSHADER);
		defLightShader = App::resources->Reference(lightPass);
	}

	return defaultShader && defaultScaleShader && skyboxShader && defGeoShader && defLightShader;
}

bool RE_InternalResources::InitMaterial()
{
	RE_Material* defMaterial = new RE_Material();
	defMaterial->SetName("Default Material");
	defMaterial->cDiffuse.x = 1.0;
	defMaterial->ProcessMD5();
	defMaterial->SetInternal(true);
	defMaterial->LoadInMemory();
	return defaultMaterial = App::resources->Reference(defMaterial);
}

bool RE_InternalResources::InitSkyBox()
{
	RE_SkyBox* rdefaultSkybox = new RE_SkyBox();
	rdefaultSkybox->SetName("defaultSkyBox");
	rdefaultSkybox->SetType(Resource_Type::R_SKYBOX);
	rdefaultSkybox->AddTexturePath(RE_TextureFace::RE_RIGHT, "Settings/DefaultAssets/Skybox/1right.dds");
	rdefaultSkybox->AddTexturePath(RE_TextureFace::RE_LEFT, "Settings/DefaultAssets/Skybox/2left.dds");
	rdefaultSkybox->AddTexturePath(RE_TextureFace::RE_TOP, "Settings/DefaultAssets/Skybox/3top.dds");
	rdefaultSkybox->AddTexturePath(RE_TextureFace::RE_BOTTOM, "Settings/DefaultAssets/Skybox/4bottom.dds");
	rdefaultSkybox->AddTexturePath(RE_TextureFace::RE_FRONT, "Settings/DefaultAssets/Skybox/5front.dds");
	rdefaultSkybox->AddTexturePath(RE_TextureFace::RE_BACK, "Settings/DefaultAssets/Skybox/6back.dds");
	rdefaultSkybox->SetAsInternal();

	return defaultSkybox = App::resources->Reference(rdefaultSkybox);
}

const char* RE_InternalResources::GetDefaultShader() const
{
	static const char* shaders[4] = { defaultShader, defaultShader, defaultShader /* TODO RUB: add shader with light input*/, defGeoShader };
	return shaders[ModuleRenderer3D::GetLightMode()];
}

const char*	 RE_InternalResources::GetDefaultScaleShader() const { return defaultScaleShader; }
const char*	 RE_InternalResources::GetDefaulMaterial() const { return defaultMaterial; }
const char*	 RE_InternalResources::GetDefaultSkyBox() const { return defaultSkybox; }
const char*	 RE_InternalResources::GetLightPassShader() const { return defLightShader; }
const char*	 RE_InternalResources::GetDefaultSkyBoxShader() const { return skyboxShader; }
unsigned int RE_InternalResources::GetTextureChecker() const { return checkerTexture; }