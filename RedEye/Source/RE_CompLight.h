#ifndef __RE_COMPLIGHT_H__
#define __RE_COMPLIGHT_H__

#include "RE_Component.h"
#include "MathGeoLib/include/Math/float3.h"

enum LightType : int
{
	L_DIRECTIONAL = 0,
	L_POINT,
	L_SPOTLIGHT
};

class RE_CompLight : public RE_Component
{
public:

	RE_CompLight();
	~RE_CompLight();

	void SetUp(RE_GameObject* parent);
	void SetUp(const RE_CompLight& cmpLight, RE_GameObject* parent);

	LightType GetType() const;

	void DrawProperties() override;

	void SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources) override;
	void DeserializeJson(JSONNode* node, eastl::map<int, const char*>* resources, RE_GameObject* parent) override;

	unsigned int GetBinarySize()const override;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) override;
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources, RE_GameObject* parent)override;

public:

	LightType type = L_DIRECTIONAL;

	// Attenuattion
	float intensity = 1.0f;
	float constant = 1.0f;
	float linear = 0.7f;
	float quadratic = 1.8f;

	// colors
	math::vec ambient; // 0.1
	math::vec diffuse; // 0.8
	math::vec specular; // 1.0

	// Spotlight
	float cutOff; // cos(radians(12.5f))
	float outerCutOff; // cos(radians(17.5f))
};

#endif // !__RE_COMPLIGHT_H__
