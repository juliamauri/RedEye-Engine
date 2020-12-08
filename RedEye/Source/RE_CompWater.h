#ifndef __RE_COMPWATER_H__
#define __RE_COMPWATER_H__

#include "RE_Component.h"
#include "Cvar.h"

#include "MathGeoLib/include/Geometry/AABB.h"
#include "MathGeoLib/include/Geometry/Ray.h"
#include "MathGeoLib/include/Math/float2.h"
#include "MathGeoLib/include/Math/float3.h"

#include <EASTL/vector.h>

class RE_CompWater : public RE_Component
{
public:
	RE_CompWater();
	~RE_CompWater() {}

	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const UID parent) override;

	void Draw() const override;
	void DrawProperties() override;

	void SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources) const override;
	void DeserializeJson(JSONNode* node, eastl::map<int, const char*>* resources) override;

	unsigned int GetBinarySize() const override;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const override;
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources) override;

	math::AABB GetAABB() const;
	bool CheckFaceCollision(const math::Ray& local_ray, float& distance) const;

	void UseResources()override;
	void UnUseResources()override;

	unsigned int GetVAO()const;
	unsigned int GetTriangles()const;

private:
	void GeneratePlane();
	void SetUpWaterUniforms();

private:
	math::AABB box;
	unsigned int VAO = 0, VBO = 0, EBO = 0, triangle_count = 0;
	int slices = 20, stacks = 20, target_slices = 0, target_stacks = 0;

	eastl::pair<ShaderCvar*, unsigned int> waterFoam = {nullptr, 0};
	eastl::vector<ShaderCvar> waterUniforms;
	eastl::pair<ShaderCvar*, float> waveLenght = {nullptr, 1.5f};
	eastl::pair<ShaderCvar*, float> amplitude = { nullptr, 0.75f };
	eastl::pair<ShaderCvar*, float> speed = { nullptr, 1.0f };
	eastl::pair<ShaderCvar*, bool> is_linear = { nullptr, true };
	eastl::pair<ShaderCvar*, math::float2> direction = { nullptr, {1.0f, 1.0f } };
	eastl::pair<ShaderCvar*, math::float2> center = { nullptr, {0.0f, 0.0f } };
	eastl::pair<ShaderCvar*, float> steepness = { nullptr, 0.7f };
	eastl::pair<ShaderCvar*, int> numWaves = { nullptr, 1 };
	eastl::pair<ShaderCvar*, math::vec> cdiffuse = { nullptr, {0.0f, 0.0f, 1.0f} };
	eastl::pair<ShaderCvar*, float> shininess = { nullptr, 1.f };
	eastl::pair<ShaderCvar*, float> foamMin = { nullptr, 0.65f };
	eastl::pair<ShaderCvar*, float> foamMax = { nullptr, 1.0f };
	eastl::pair<ShaderCvar*, math::vec> foam_color = { nullptr, math::vec::one };
	eastl::pair<ShaderCvar*, float> opacity = { nullptr, 1.f };
	eastl::pair<ShaderCvar*, float> distanceFoam = { nullptr, 0.0002f };
};

#endif // !__RE_COMPWATER_H__