#ifndef __RE_MATERIAL_H__
#define __RE_MATERIAL_H__

#include "Resource.h"
#include "Cvar.h"

#include "MathGeoLib/include/Math/float3.h"
#include <string>
#include <vector>

class JSONNode;

enum RE_ShadingMode { //from assimp documentation
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

	void UseTextureResources();
	void UnUseTextureResources();

	void UploadToShader(float* model, bool usingChekers);

	unsigned int GetShaderID()const;

	void DrawMaterialEdit();

private:
	void Draw() override;

	void SaveResourceMeta(JSONNode* metaNode)override;
	void LoadResourceMeta(JSONNode* metaNode)override;

	void DrawTextures(const char* texturesName, std::vector<const char*>* textures);

	void JsonDeserialize(bool generateLibraryPath = false);
	void JsonSerialize(bool onlyMD5 = false);

	void PullTexturesJson(JSONNode * texturesNode, std::vector<const char*>* textures);
	void PushTexturesJson(JSONNode * texturesNode, std::vector<const char*>* textures);

	void BinaryDeserialize();
	void BinarySerialize();
	unsigned int GetBinarySize();

	void GetAndProcessUniformsFromShader();

public:
	RE_ShadingMode shadingType = S_FLAT;

	std::vector<const char*> tDiffuse;
	math::float3 cDiffuse = math::float3::zero;
	std::vector<const char*> tSpecular;
	math::float3 cSpecular = math::float3::zero;
	std::vector<const char*> tAmbient;
	math::float3 cAmbient = math::float3::zero;
	std::vector<const char*> tEmissive;
	math::float3 cEmissive = math::float3::zero;
	math::float3 cTransparent = math::float3::zero;

	bool backFaceCulling = true;
	bool blendMode = false;

	std::vector<const char*> tOpacity;
	float opacity = 1.0f;
	std::vector<const char*> tShininess;
	float shininess = 0.f;
	float shininessStrenght = 1.0f;
	float refraccti = 1.0f;

	std::vector<const char*> tHeight;
	std::vector<const char*> tNormals;
	std::vector<const char*> tReflection;
	std::vector<const char*> tUnknown;

private:
	const char* shadingItems[10] = { "Flat", "Goraund", "Phong", "Phong Blinn", "Toon", "Oren Nayar", "Minnaert", "Cook Torrance", "No Shading", "Fresnel" };

	bool applySave = false;

	const char* shaderMD5 = nullptr;
	std::vector<ShaderCvar> fromShaderCustomUniforms;

	enum MaterialUINT {
		UNDEFINED = -1, CDIFFUSE, TDIFFUSE, CSPECULAR, TSPECULAR, CAMBIENT, TAMBIENT, CEMISSIVE,
		TEMISSIVE, CTRANSPARENT, OPACITY, TOPACITY, SHININESS, SHININESSSTRENGHT, TSHININESS, REFRACCTI,
		THEIGHT, TNORMALS, TREFLECTION
	};
	unsigned int usingOnMat[18] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
};

#endif // !__RE_MATERIAL_H__