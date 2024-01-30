#ifndef __RE_MATERIAL_H__
#define __RE_MATERIAL_H__

#include "Resource.h"
#include "RE_Cvar.h"

#include <MGL/Math/float3.h>
#include <EASTL/vector.h>

class RE_Json;

class RE_Material : public ResourceContainer
{
public:

	enum class ShadingMode : int // from assimp documentation
	{
		FLAT = 0x1,
		GORAUND,
		PHONG,
		PHONG_BLINN,
		TOON,
		ORENNAYAR,
		MINNAERT,
		COOKTORRANCE,
		NOSHADING, //No shading at all. Constant light influence of 1.0.
		FRESNEL
	};

	RE_Material() = default;
	RE_Material(const char* metapath) : ResourceContainer(metapath) {}
	~RE_Material() final = default;

	void LoadInMemory() final override;
	void UnloadMemory() final override;

	void Import(bool keepInMemory = true) final override;

	void ProcessMD5();

	void Save();

	void UseResources();
	void UnUseResources();

	void UploadToShader(const float* model, bool usingChekers, bool defaultShader = false);
	void UploadAsParticleDataToShader(uint shaderID, bool useTextures, bool lighting);

	uint GetShaderID() const;

	void DrawMaterialEdit();
	void DrawMaterialParticleEdit(bool tex);

	void SomeResourceChanged(const char* resMD5) final override;

	bool ExistsOnShader(const char* shader) const;
	bool ExistsOnTexture(const char* texture) const;

	//direct method, use it when creates the material.
	void SetShader(const char* shaderMD5);

	void DeleteShader();
	void DeleteTexture(const char* texMD5);

private:

	void Draw() final override;

	void SaveResourceMeta(RE_Json* metaNode) const final override;
	void LoadResourceMeta(RE_Json* metaNode) final override;

	bool NeededResourcesReferenced(RE_Json* metaNode) final override;

	void DrawTextures(const char* texturesName, eastl::vector<const char*>* textures);

	void JsonDeserialize(bool generateLibraryPath = false);
	void JsonSerialize(bool onlyMD5 = false);

	void PullTexturesJson(RE_Json * texturesNode, eastl::vector<const char*>* textures);
	void PushTexturesJson(RE_Json * texturesNode, const eastl::vector<const char*>* textures) const;

	bool NeededResourcesReferencedTexturePull(RE_Json* texturesNode) const;

	void BinaryDeserialize();
	void BinarySerialize();
	size_t GetBinarySize() const;

	void GetAndProcessUniformsFromShader();

	bool ExistsOnTexture(const char* texture, const eastl::vector<const char*>* textures) const;
	void DeleteTexture(const char* texMD5, eastl::vector<const char*>* textures);

public:

	ShadingMode shadingType = ShadingMode::FLAT;

	eastl::vector<const char*> tDiffuse;
	math::float3 cDiffuse = math::float3::zero;
	eastl::vector<const char*> tSpecular;
	math::float3 cSpecular = math::float3::zero;
	eastl::vector<const char*> tAmbient;
	math::float3 cAmbient = math::float3::zero;
	eastl::vector<const char*> tEmissive;
	math::float3 cEmissive = math::float3::zero;
	math::float3 cTransparent = math::float3::zero;

	bool backFaceCulling = true;
	bool blendMode = false;

	eastl::vector<const char*> tOpacity;
	float opacity = 1.f;
	eastl::vector<const char*> tShininess;
	float shininess = 1.f;
	float shininessStrenght = 1.f;
	float refraccti = 1.f;

	eastl::vector<const char*> tHeight;
	eastl::vector<const char*> tNormals;
	eastl::vector<const char*> tReflection;
	eastl::vector<const char*> tUnknown;

private:

	//const char* shadingItems[10] = { "Flat", "Goraund", "Phong", "Phong Blinn", "Toon", "Oren Nayar", "Minnaert", "Cook Torrance", "No Shading", "Fresnel" };

	bool applySave = false;

	const char* shaderMD5 = nullptr;
	eastl::vector<RE_Shader_Cvar> fromShaderCustomUniforms;

	enum class MaterialUINT : short
	{
		UNDEFINED = -1,

		CDIFFUSE,
		TDIFFUSE,
		CSPECULAR,
		TSPECULAR,
		CAMBIENT,
		TAMBIENT,
		CEMISSIVE,

		TEMISSIVE,
		CTRANSPARENT,
		OPACITY,
		TOPACITY,
		SHININESS,
		SHININESSSTRENGHT,
		TSHININESS,
		REFRACCTI,
		
		THEIGHT,
		TNORMALS,
		TREFLECTION
	};

	ushort usingOnMat[18] = { 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0 };
};

#endif // !__RE_MATERIAL_H__