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
		float near_plane = 1.0f,
		float far_plane = 5000.0f,
		float v_fov = 0.523599f,
		RE_Camera::AspectRatio ar = RE_Camera::AspectRatio::Fit_Window,
		bool draw_frustum = true,
		bool usingSkybox = true,
		const char* skyboxMD5 = nullptr);

	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent) final;
	
	void Update() final;
	void OnTransformModified() final;
	void DrawProperties() final;

	// Getters
	class RE_CompTransform* GetTransform() const;

	// Skybox
	void DrawSkybox() const;
	bool isUsingSkybox() const;
	const char* GetSkybox() const;
	void DeleteSkybox();
	void SetSkyBox(const char* md5);

	// Resources - Skybox
	void UseResources() final;
	void UnUseResources() final;
	eastl::vector<const char*> GetAllResources() final;

	// Serialization
	void SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources) const final;
	void DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources) final;

	size_t GetBinarySize() const final;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const final;
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources) final;

public:

	RE_Camera Camera;

	bool draw_frustum = true;
	bool override_cull = false;

private:

	bool need_recalculation = true;

	// Skybox
	bool usingSkybox = true;
	const char* skyboxMD5 = nullptr;
};

#endif // !__RE_COMPCAMERA_H__