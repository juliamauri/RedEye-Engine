#include "RE_InternalResources.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "RE_ConsoleLog.h"
#include "RE_FileBuffer.h"
#include "RE_ResourceManager.h"
#include "RE_GLCacheManager.h"
#include "RE_ShaderImporter.h"
#include "RE_TextureImporter.h"

#include "RE_DefaultShaders.h"
#include "RE_Shader.h"
#include "RE_Material.h"
#include "RE_SkyBox.h"

#include "Glew/include/glew.h"
#include <gl/GL.h>

using namespace RE_InternalResources::Internal;

void RE_InternalResources::Init()
{
	InitChecker();
	if (!InitShaders()) RE_LOG_WARNING("Could not initialize default shaders");
	InitWaterResources();
	if (!InitMaterial()) RE_LOG_WARNING("Could not initialize default materials");
	if (!InitSkyBox()) RE_LOG_WARNING("Could not initialize default skybox");
}

void RE_InternalResources::Clear()
{
	if (checkerTexture != 0) glDeleteTextures(1, &checkerTexture);
	if (water_foam_texture != 0) glDeleteTextures(1, &water_foam_texture);
}

const char* RE_InternalResources::GetDefaultShader()
{
	static const char* shaders[4] = { defaultShader, defaultShader, defaultShader /* TODO RUB: add shader with light input*/, defGeoShader };
	return shaders[ModuleRenderer3D::GetLightMode()];
}

const char* RE_InternalResources::GetDefaultWaterShader()
{
	static const char* waterShaders[4] = { waterShader, waterShader, waterShader /* TODO RUB: add shader with light input*/, waterDefShader };
	return waterShaders[ModuleRenderer3D::GetLightMode()];
}

const char* RE_InternalResources::GetDefaultScaleShader() { return defaultScaleShader; }
const char* RE_InternalResources::GetDefaulMaterial() { return defaultMaterial; }
const char* RE_InternalResources::GetDefaultSkyBox() { return defaultSkybox; }
const char* RE_InternalResources::GetLightPassShader() { return defLightShader; }
const char* RE_InternalResources::GetDefaultSkyBoxShader() { return skyboxShader; }
unsigned int RE_InternalResources::GetTextureChecker() { return checkerTexture; }
unsigned int RE_InternalResources::GetTextureWaterFoam() { return water_foam_texture; }
eastl::vector<RE_Shader_Cvar> RE_InternalResources::GetWaterUniforms() { return waterUniforms; }

void RE_InternalResources::Internal::InitChecker()
{
	// Checkers
	unsigned char imageData[264][264][3];
	int IMAGE_ROWS = 264, IMAGE_COLS = 264;
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

bool RE_InternalResources::Internal::InitShaders()
{
	//Loading Shaders
	// Default
	RE_Shader* defSRes = new RE_Shader();
	defSRes->SetName("Default Shader");
	defSRes->SetType(Resource_Type::R_SHADER);
	defSRes->SetAsInternal(DEFVERTEXSHADER, DEFFRAGMENTSHADER);
	defaultShader = RE_ResourceManager::Reference(defSRes);

	// Scaled (for outline)
	RE_Shader* defScaleRes = new RE_Shader();
	defScaleRes->SetName("Default Scale Shader");
	defScaleRes->SetType(Resource_Type::R_SHADER);
	defScaleRes->SetAsInternal(DEFVERTEXSCALESHADER, DEFFRAGMENTSHADER);
	defaultScaleShader = RE_ResourceManager::Reference(defScaleRes);

	// Skybox
	RE_Shader* defSKRes = new RE_Shader();
	defSKRes->SetName("Default SkyBox Shader");
	defSKRes->SetType(Resource_Type::R_SHADER);
	defSKRes->SetAsInternal(SKYBOXVERTEXSHADER, SKYBOXFRAGMENTSHADER);
	skyboxShader = RE_ResourceManager::Reference(defSKRes);

	// Deferred
	RE_Shader* deferred = new RE_Shader();
	deferred->SetName("Deferred Shader");
	deferred->SetType(Resource_Type::R_SHADER);
	deferred->SetAsInternal(GEOPASSVERTEXSHADER, GEOPASSFRAGMENTSHADER);
	defGeoShader = RE_ResourceManager::Reference(deferred);

	// Light Pass
	RE_Shader* lightPass = new RE_Shader();
	lightPass->SetName("Light Pass Shader");
	lightPass->SetType(Resource_Type::R_SHADER);
	lightPass->SetAsInternal(LIGHTPASSVERTEXSHADER, LIGHTPASSFRAGMENTSHADER);
	defLightShader = RE_ResourceManager::Reference(lightPass);

	return defaultShader && defaultScaleShader && skyboxShader && defGeoShader && defLightShader;
}

bool RE_InternalResources::Internal::InitMaterial()
{
	RE_Material* defMaterial = new RE_Material();
	defMaterial->SetName("Default Material");
	defMaterial->cDiffuse.x = 1.0;
	defMaterial->ProcessMD5();
	defMaterial->SetInternal(true);
	defMaterial->LoadInMemory();
	return defaultMaterial = RE_ResourceManager::Reference(defMaterial);
}

bool RE_InternalResources::Internal::InitSkyBox()
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

	return defaultSkybox = RE_ResourceManager::Reference(rdefaultSkybox);
}

void RE_InternalResources::Internal::InitWaterResources()
{
	// Deferred
	RE_Shader* waterSr = new RE_Shader();
	waterSr->SetName("Water Shader");
	waterSr->SetType(Resource_Type::R_SHADER);
	waterSr->SetAsInternal(WATERVERTEXSHADER, WATERFRAGMENTSHADER);
	waterShader = RE_ResourceManager::Reference(waterSr);

	// Light Pass
	RE_Shader* waterDefS = new RE_Shader();
	waterDefS->SetName("Water Deferred Shader");
	waterDefS->SetType(Resource_Type::R_SHADER);
	waterDefS->SetAsInternal(WATERPASSVERTEXSHADER, WATERPASSFRAGMENTSHADER);
	waterDefShader = RE_ResourceManager::Reference(waterDefS);


	static const char* internalNames[30] = { "useTexture", "useColor", "useClipPlane", "clip_plane", "time", "dt", "near_plane", "far_plane", "viewport_w", "viewport_h", "model", "view", "projection", "tdiffuse", "cspecular", "tspecular", "cambient", "tambient", "cemissive", "temissive", "ctransparent", "topacity", "tshininess", "shininessST", "refraccti", "theight", "tnormals", "treflection", "currentDepth", "viewPos" };
	eastl::vector<RE_Shader_Cvar> uniformsWaterShader = waterSr->GetUniformValues(), uniformsWaterDefShader = waterDefS->GetUniformValues();
	for (unsigned int i = 0; i < uniformsWaterShader.size(); i++) {
		bool skip = false;
		for (int iN = 0; iN < 30 && !skip; iN++)
			if (uniformsWaterShader[i].name == internalNames[iN])
				skip = true;
		if (skip) continue;
		
		for (unsigned int j = 0; j < uniformsWaterDefShader.size(); j++) {
			if (uniformsWaterShader[i].name == uniformsWaterDefShader[j].name) {
				uniformsWaterShader[i].locationDeferred = uniformsWaterDefShader[j].location;
				waterUniforms.push_back(uniformsWaterShader[i]);
				break;
			}
		}
	}

	RE_FileBuffer waterTexture(WATER_FOAM_TEX_PATH);
	if (waterTexture.Load())
	{
		RE_TextureSettings defTexSettings;
		int tmp1, tmp2;
		RE_TextureImporter::LoadTextureInMemory(waterTexture.GetBuffer(), waterTexture.GetSize(), TextureType::RE_PNG, &water_foam_texture, &tmp1, &tmp2, defTexSettings);
	}
}