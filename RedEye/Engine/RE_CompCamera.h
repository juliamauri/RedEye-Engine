#ifndef __RE_COMPCAMERA_H__
#define __RE_COMPCAMERA_H__

#include "RE_Component.h"
#include "RE_Camera.h"

class RE_CompCamera : public RE_Component
{
public:

	RE_CompCamera::RE_CompCamera() : RE_Component(RE_Component::Type::CAMERA) {}
	~RE_CompCamera() final = default;

	void SetProperties(
		math::float2 bounds = { 300.f, 300.f },
		AspectRatio ar = AspectRatio::Fit_Window,
		float v_fov = 0.523599f,
		float near_plane = 1.0f,
		float far_plane = 5000.0f,
		bool usingSkybox = true,
		const char* skyboxMD5 = nullptr,
		bool draw_frustum = true);

	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent) final;
	
	void Update() final;
	void OnTransformModified() final { need_recalculation = true; }
	void DrawProperties() final;

	// Getters
	class RE_CompTransform* GetTransform() const;

	// Resources - Skybox
	void UseResources() final { Camera.UseSkybox(); }
	void UnUseResources() final { Camera.UnUseSkybox(); }
	eastl::vector<const char*> GetAllResources() const final;

	// Serialization
	void JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources = nullptr) const final;
	void JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources = nullptr) final;

	size_t GetBinarySize() const final;
	void BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources = nullptr) const final;
	void BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources = nullptr) final;

public:

	RE_Camera Camera;

	bool draw_frustum = false;
	bool override_cull = false;

private:

	bool need_recalculation = true;
};

#endif // !__RE_COMPCAMERA_H__