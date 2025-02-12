#ifndef __RE_COMPLIGHT_H__
#define __RE_COMPLIGHT_H__

#include "RE_Component.h"
#include "RE_DataTypes.h"

class RE_CompLight : public RE_Component
{
public:

	RE_CompLight();
	~RE_CompLight() final = default;

	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent) final;

	void CallShaderUniforms(unsigned int shader, const char* unif_name) const;

	void DrawProperties() final;

	size_t GetBinarySize() const final;
	void SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources) const final;
	void DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources) final;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const final;
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources) final;

private:

	inline void UpdateCutOff();

public:

	enum class Type : ushort
	{
		DIRECTIONAL = 0,
		POINT,
		SPOTLIGHT
	};
	
	Type light_type = Type::POINT;

	// Attenuattion
	float intensity = 1.0f;
	float constant = 1.0f;
	float linear = 0.091f;
	float quadratic = 0.011f;

	// color
	math::vec diffuse; // 0.8
	float specular = 0.2f;

	// Spotlight
	float cutOff[2]; // cos(radians(12.5f))
	float outerCutOff[2]; // cos(radians(17.5f))
};

#endif // !__RE_COMPLIGHT_H__
