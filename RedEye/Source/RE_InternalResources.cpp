#include "RE_InternalResources.h"

#include "RE_Profiler.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "RE_ConsoleLog.h"
#include "RE_FileBuffer.h"
#include "RE_ResourceManager.h"
#include "RE_GLCache.h"
#include "RE_ShaderImporter.h"
#include "RE_TextureImporter.h"

#include "RE_DefaultShaders.h"
#include "RE_Shader.h"
#include "RE_Material.h"
#include "RE_SkyBox.h"

#include "Glew/include/glew.h"
#include <gl/GL.h>

#define WATER_FOAM_TEX_PATH "Settings/DefaultAssets/water_foam.png"

void RE_InternalResources::Init()
{
	RE_PROFILE(PROF_Init, PROF_InternalResources);
	InitChecker();
	if (!InitShaders()) RE_LOG_WARNING("Could not initialize default shaders");
	InitWaterResources();
	if (!InitMaterial()) RE_LOG_WARNING("Could not initialize default materials");
	if (!InitSkyBox()) RE_LOG_WARNING("Could not initialize default skybox");
}

void RE_InternalResources::Clear()
{
	RE_PROFILE(PROF_Clear, PROF_InternalResources);
	if (checkerTexture != 0u) glDeleteTextures(1, &checkerTexture);
	if (water_foam_texture != 0u) glDeleteTextures(1, &water_foam_texture);
}

void RE_InternalResources::InitChecker()
{
	RE_PROFILE(PROF_InitChecker, PROF_InternalResources);
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
	RE_GLCache::ChangeTextureBind(checkerTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, IMAGE_COLS, IMAGE_ROWS, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}

bool RE_InternalResources::InitShaders()
{
	RE_PROFILE(PROF_InitShaders, PROF_InternalResources);
	//Loading Shaders
	// Default
	RE_Shader* defSRes = new RE_Shader();
	defSRes->SetName("Default Shader");
	defSRes->SetType(Resource_Type::R_SHADER);
	defSRes->SetAsInternal(DEFVERTEXSHADER, DEFFRAGMENTSHADER);
	defaultShader = RE_RES->Reference(defSRes);

	// Scaled (for outline)
	RE_Shader* defScaleRes = new RE_Shader();
	defScaleRes->SetName("Default Scale Shader");
	defScaleRes->SetType(Resource_Type::R_SHADER);
	defScaleRes->SetAsInternal(DEFVERTEXSCALESHADER, DEFFRAGMENTSHADER);
	defaultScaleShader = RE_RES->Reference(defScaleRes);

	// Skybox
	RE_Shader* defSKRes = new RE_Shader();
	defSKRes->SetName("Default SkyBox Shader");
	defSKRes->SetType(Resource_Type::R_SHADER);
	defSKRes->SetAsInternal(SKYBOXVERTEXSHADER, SKYBOXFRAGMENTSHADER);
	skyboxShader = RE_RES->Reference(defSKRes);

	// Deferred
	RE_Shader* deferred = new RE_Shader();
	deferred->SetName("Deferred Shader");
	deferred->SetType(Resource_Type::R_SHADER);
	deferred->SetAsInternal(GEOPASSVERTEXSHADER, GEOPASSFRAGMENTSHADER);
	defGeoShader = RE_RES->Reference(deferred);

	// Light Pass
	RE_Shader* lightPass = new RE_Shader();
	lightPass->SetName("Light Pass Shader");
	lightPass->SetType(Resource_Type::R_SHADER);
	lightPass->SetAsInternal(LIGHTPASSVERTEXSHADER, LIGHTPASSFRAGMENTSHADER);
	defLightShader = RE_RES->Reference(lightPass);

	// Particle Light Pass
	RE_Shader* lightParticlePass = new RE_Shader();
	lightParticlePass->SetName("Particle Light Pass Shader");
	lightParticlePass->SetType(Resource_Type::R_SHADER);
	lightParticlePass->SetAsInternal(LIGHTPASSVERTEXSHADER, PARTICLELIGHTPASSFRAGMENTSHADER);
	defParticleLightShader = RE_RES->Reference(lightParticlePass);

	// Particle
	RE_Shader* particleS = new RE_Shader();
	particleS->SetName("Particle Shader");
	particleS->SetType(Resource_Type::R_SHADER);
	particleS->SetAsInternal(PARTICLEVERTEXSHADER, PARTICLEFRAGMENTSHADER);
	particleShader = RE_RES->Reference(particleS);

	// Deferred Particle
	RE_Shader* defParticleS = new RE_Shader();
	defParticleS->SetName("Geo Pass Particle Shader");
	defParticleS->SetType(Resource_Type::R_SHADER);
	defParticleS->SetAsInternal(PARTICLEGEOPASSVERTEXSHADER, PARTICLEGEOPASSFRAGMENTSHADER);
	defParticleShader = RE_RES->Reference(defParticleS);

	return defaultShader && defaultScaleShader && skyboxShader && defGeoShader && defLightShader && particleShader && defParticleShader;
}

bool RE_InternalResources::InitMaterial()
{
	RE_PROFILE(PROF_InitMaterial, PROF_InternalResources);
	RE_Material* defMaterial = new RE_Material();
	defMaterial->SetName("Default Material");
	defMaterial->cDiffuse.x = 1.0;
	defMaterial->ProcessMD5();
	defMaterial->SetInternal(true);
	defMaterial->LoadInMemory();
	return defaultMaterial = RE_RES->Reference(defMaterial);
}

bool RE_InternalResources::InitSkyBox()
{
	RE_PROFILE(PROF_InitSkyBox, PROF_InternalResources);
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

	return defaultSkybox = RE_RES->Reference(rdefaultSkybox);
}

void RE_InternalResources::InitWaterResources()
{
	RE_PROFILE(PROF_InitWater, PROF_InternalResources);
	// Deferred
	RE_Shader* waterSr = new RE_Shader();
	waterSr->SetName("Water Shader");
	waterSr->SetType(Resource_Type::R_SHADER);
	waterSr->SetAsInternal(WATERVERTEXSHADER, WATERFRAGMENTSHADER);
	waterShader = RE_RES->Reference(waterSr);

	// Light Pass
	RE_Shader* waterDefS = new RE_Shader();
	waterDefS->SetName("Water Deferred Shader");
	waterDefS->SetType(Resource_Type::R_SHADER);
	waterDefS->SetAsInternal(WATERPASSVERTEXSHADER, WATERPASSFRAGMENTSHADER);
	waterDefShader = RE_RES->Reference(waterDefS);


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

const char* RE_InternalResources::GetDefaultShader() const
{
	static const char* shaders[4] = { defaultShader, defaultShader, defaultShader /* TODO RUB: add shader with light input*/, defGeoShader };
	return shaders[ModuleRenderer3D::GetLightMode()];
}

const char* RE_InternalResources::GetDefaultWaterShader() const
{
	static const char* waterShaders[4] = { waterShader, waterShader, waterShader /* TODO RUB: add shader with light input*/, waterDefShader };
	return waterShaders[ModuleRenderer3D::GetLightMode()];
}

const char*	 RE_InternalResources::GetDefaultScaleShader() const { return defaultScaleShader; }
const char*	 RE_InternalResources::GetDefaulMaterial() const { return defaultMaterial; }
const char*	 RE_InternalResources::GetDefaultSkyBox() const { return defaultSkybox; }
const char*	 RE_InternalResources::GetLightPassShader() const { return defLightShader; }

const char* RE_InternalResources::GetParticleLightPassShader() const { return defParticleLightShader; }

const char* RE_InternalResources::GetParticleShader() const
{
	static const char* particleshaders[4] = { particleShader, particleShader, particleShader /* TODO RUB: add shader with light input*/, defParticleShader };
	return particleshaders[ModuleRenderer3D::GetLightMode()];
}

const char*	 RE_InternalResources::GetDefaultSkyBoxShader() const { return skyboxShader; }
unsigned int RE_InternalResources::GetTextureChecker() const { return checkerTexture; }
unsigned int RE_InternalResources::GetTextureWaterFoam() const { return water_foam_texture; }
eastl::vector<RE_Shader_Cvar> RE_InternalResources::GetWaterUniforms() const { return waterUniforms; }
