#ifndef __RE_MATERIAL_H__
#define __RE_MATERIAL_H__

class RE_Json;

enum RE_ShadingMode : int { //from assimp documentation
	S_FLAT = 0x1,
	S_GORAUND,
	S_PHONG,
	S_PHONG_BLINN,
	S_TOON,
	S_ORENNAYAR,
	S_MINNAERT,
	S_COOKTORRANCE,
	S_NOSHADING, //No shading at all. Constant light influence of 1.0.
	S_FRESNEL
};

class RE_Material : public ResourceContainer
{
public:

	RE_Material();
	RE_Material(const char* metapath);
	~RE_Material();

	void LoadInMemory() override;
	void UnloadMemory() override;

	void Import(bool keepInMemory = true) override;

	void ProcessMD5();

	void Save();

	void UseResources();
	void UnUseResources();

	void UploadToShader(const float* model, bool usingChekers, bool defaultShader = false);
	void UploadAsParticleDataToShader(unsigned int shaderID, bool useTextures, bool lighting);

	unsigned int GetShaderID()const;

	void DrawMaterialEdit();
	void DrawMaterialParticleEdit(bool tex);

	void SomeResourceChanged(const char* resMD5) override;

	bool ExistsOnShader(const char* shader) const;
	bool ExistsOnTexture(const char* texture) const;

	//direct method, use it when creates the material.
	void SetShader(const char* shaderMD5);

	void DeleteShader();
	void DeleteTexture(const char* texMD5);

private:

	void Draw() override;

	void SaveResourceMeta(RE_Json* metaNode)override;
	void LoadResourceMeta(RE_Json* metaNode)override;

	bool NeededResourcesReferenced(RE_Json* metaNode) override;

	void DrawTextures(const char* texturesName, eastl::vector<const char*>* textures);

	void JsonDeserialize(bool generateLibraryPath = false);
	void JsonSerialize(bool onlyMD5 = false);

	void PullTexturesJson(RE_Json * texturesNode, eastl::vector<const char*>* textures);
	void PushTexturesJson(RE_Json * texturesNode, eastl::vector<const char*>* textures);

	bool NeededResourcesReferencedTexturePull(RE_Json* texturesNode);

	void BinaryDeserialize();
	void BinarySerialize();
	size_t GetBinarySize();

	void GetAndProcessUniformsFromShader();

	bool ExistsOnTexture(const char* texture, const eastl::vector<const char*>* textures) const;
	void DeleteTexture(const char* texMD5, eastl::vector<const char*>* textures);

public:

	RE_ShadingMode shadingType = S_FLAT;

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

	enum MaterialUINT {
		UNDEFINED = -1, CDIFFUSE, TDIFFUSE, CSPECULAR, TSPECULAR, CAMBIENT, TAMBIENT, CEMISSIVE,
		TEMISSIVE, CTRANSPARENT, OPACITY, TOPACITY, SHININESS, SHININESSSTRENGHT, TSHININESS, REFRACCTI,
		THEIGHT, TNORMALS, TREFLECTION
	};
	unsigned int usingOnMat[18] = { 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0 };
};

#endif // !__RE_MATERIAL_H__