#ifndef __RE_COMPPARTICLEEMITER_H__
#define __RE_COMPPARTICLEEMITER_H__

#include "RE_Component.h"
#include "MathGeoLib/include/Math/float3.h"
#include <vector>

class Particle;
class RE_Mesh;
struct RE_ParticleEmitter;
class RE_CompPrimitive;

class RE_CompParticleEmitter : public RE_Component
{
public:
	RE_CompParticleEmitter();
	~RE_CompParticleEmitter();

	void AddSimulation();

	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const UID parent) override;

	void Update() override;

	void Draw() const override;
	void DrawProperties() override;

	void SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources) const override;
	void DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources) override;

	unsigned int GetBinarySize() const override;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const override;
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources) override;

	void UseResources();
	void UnUseResources();

	bool isLighting() const;
	void CallLightShaderUniforms(unsigned int shader, const char* array_unif_name, unsigned int& count, unsigned int maxLights, bool sharedLight) const;

private:

	RE_ParticleEmitter* simulation = nullptr;

	bool draw = false;

	// Triangle "Point"
	unsigned int VAO = 0u, VBO = 0u, EBO = 0u;
};

#endif // !__RE_COMPPARTICLEEMITER_H__