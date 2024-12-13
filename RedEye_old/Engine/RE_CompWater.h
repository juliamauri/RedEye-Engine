#ifndef __RE_COMPWATER_H__
#define __RE_COMPWATER_H__

class RE_CompWater : public RE_Component
{
public:
	RE_CompWater();
	~RE_CompWater() final = default;

	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent) final;

	void Draw() const final;
	void DrawProperties() final;

	void SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources) const final;
	void DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources) final;

	size_t GetBinarySize() const final;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const final;
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources) final;

	math::AABB GetAABB() const;
	bool CheckFaceCollision(const math::Ray& local_ray, float& distance) const;

	void UseResources() final;
	void UnUseResources() final;

	unsigned int GetVAO() const;
	size_t GetTriangles() const;

private:

	void GeneratePlane();
	void SetUpWaterUniforms();

	void DeferedDraw(const uint texture_count) const;
	void DefaultDraw(const uint texture_count) const;

private:

	math::AABB box;

	uint VAO = 0;
	uint VBO = 0;
	uint EBO = 0;

	size_t triangle_count = 0;
	int slices = 20;
	int stacks = 20;
	int target_slices = 0;
	int target_stacks = 0;

	eastl::pair<RE_Shader_Cvar*, uint> waterFoam = { nullptr, 0 };
	eastl::vector<RE_Shader_Cvar> waterUniforms;
	eastl::pair<RE_Shader_Cvar*, float> waveLenght = { nullptr, 1.5f };
	eastl::pair<RE_Shader_Cvar*, float> amplitude = { nullptr, 0.75f };
	eastl::pair<RE_Shader_Cvar*, float> speed = { nullptr, 1.0f };
	eastl::pair<RE_Shader_Cvar*, bool> is_linear = { nullptr, true };
	eastl::pair<RE_Shader_Cvar*, math::float2> direction = { nullptr, { 1.0f, 1.0f } };
	eastl::pair<RE_Shader_Cvar*, math::float2> center = { nullptr, { 0.0f, 0.0f } };
	eastl::pair<RE_Shader_Cvar*, float> steepness = { nullptr, 0.7f };
	eastl::pair<RE_Shader_Cvar*, int> numWaves = { nullptr, 1 };
	eastl::pair<RE_Shader_Cvar*, math::vec> cdiffuse = { nullptr, { 0.0f, 0.0f, 1.0f } };
	eastl::pair<RE_Shader_Cvar*, float> shininess = { nullptr, 1.f };
	eastl::pair<RE_Shader_Cvar*, float> foamMin = { nullptr, 0.65f };
	eastl::pair<RE_Shader_Cvar*, float> foamMax = { nullptr, 1.0f };
	eastl::pair<RE_Shader_Cvar*, math::vec> foam_color = { nullptr, math::vec::one };
	eastl::pair<RE_Shader_Cvar*, float> opacity = { nullptr, 1.f };
	eastl::pair<RE_Shader_Cvar*, float> distanceFoam = { nullptr, 0.0002f };
};

#endif // !__RE_COMPWATER_H__