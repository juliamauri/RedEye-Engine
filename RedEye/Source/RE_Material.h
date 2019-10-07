#ifndef __RE_MATERIAL_H__
#define __RE_MATERIAL_H__

#include "Resource.h"

#include "RapidJson\include\rapidjson.h"
#include "RapidJson\include\document.h"
#include "RapidJson\include\allocators.h"

#include "MathGeoLib/include/Math/float3.h"
#include <string>
#include <vector>

class JSONNode;

enum RE_ShadingMode { //from assimp documentation
	S_FLAT = 0x1,
	S_GORAUND,
	S_PHONG,
	S_PHONG_BLIIN,
	S_TOON,
	S_ORENNAYAR,
	S_MINNAERT,
	S_COOKTORRANCE,
	S_NOSHADING, //No shading at all. Constant light influence of 1.0.
	S_FRESNEL
};

class RE_Material : ResourceContainer
{
public:
	RE_Material(const char* name);
	RE_Material(const char* name, rapidjson::Value* val);
	~RE_Material();

	void Serialize(JSONNode* node, rapidjson::Value* val);

public:
	std::string name;
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
};

#endif // !__RE_MATERIAL_H__