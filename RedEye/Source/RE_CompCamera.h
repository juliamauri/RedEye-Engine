#ifndef __RE_COMPCAMERA_H__
#define __RE_COMPCAMERA_H__

#include "RE_Component.h"
#include "RE_Math.h"

class RE_CompTransform;

class RE_CompCamera : public RE_Component
{
public:

	enum AspectRatioTYPE
	{
		Fit_Window = 0,
		Square_1x1,
		TraditionalTV_4x3,
		Movietone_16x9,
		Personalized
	} target_ar = Fit_Window;

public:

	RE_CompCamera();
	~RE_CompCamera();

	void SetProperties(bool toPerspective = true, float near_plane = 1.0f, float far_plane = 5000.0f, float v_fov = 0.523599f, short aspect_ratio_t = 0, bool draw_frustum = true, bool usingSkybox = true, const char* skyboxMD5 = nullptr);
	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const UID parent) override;
	
	void Update() override;
	void OnTransformModified() override;

	void DrawProperties() override;
	void DrawAsEditorProperties();
	void DrawFrustum() const;
	void DrawSkybox() const;

	// Setters
	void SetPlanesDistance(float near_plane, float far_plane);
	void SetFOV(float vertical_fov_degrees);
	void ForceFOV(float vertical_fov_degrees, float horizontal_fov_degrees);
	void SetAspectRatio(AspectRatioTYPE aspect_ratio);
	void SetBounds(float width, float height);
	void SetPerspective();
	void SetOrthographic();
	void SwapCameraType();

	// Getters
	float GetTargetWidth() const;
	float GetTargetHeight() const;
	void GetTargetWidthHeight(int &width, int &height) const;
	void GetTargetWidthHeight(float &width, float &height) const;
	void GetTargetViewPort(math::float4 &viewPort) const;
	bool isPrespective()const { return isPerspective;  }
	float GetNearPlane() const;
	float GetFarPlane() const;

	bool OverridesCulling() const;
	const math::Frustum GetFrustum() const;
	float GetVFOVDegrees() const;
	float GetHFOVDegrees() const;

	RE_CompTransform* GetTransform() const;
	math::float4x4 GetView();
	const float* GetViewPtr();
	math::float4x4 GetProjection();
	const float* GetProjectionPtr();

	// Camera Controls
	void LocalRotate(float dx, float dy);
	void LocalMove(Dir dir, float speed);
	void Orbit(float dx, float dy, math::vec center);
	void Focus(math::vec center, float radius = 1.f, float min_dist = 3.0f);

	// Skybox
	bool isUsingSkybox() const;
	const char* GetSkybox() const;
	void DeleteSkybox();
	void SetSkyBox(const char* resS) { skyboxMD5 = resS; }

	// Resources - Skybox
	void UseResources() override;
	void UnUseResources() override;
	eastl::vector<const char*> GetAllResources() override;

	// Serialization
	unsigned int GetBinarySize() const override;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const override;
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources)override;
	void SerializeJson(JSONNode* node, eastl::map<const char*, int>* resources) const override;
	void DeserializeJson(JSONNode* node, eastl::map<int, const char*>* resources) override;

private:

	void RecalculateMatrixes();
	void DrawItsProperties();

private:

	// Transform
	RE_CompTransform* transform = nullptr;

	// Axis
	math::vec right = math::vec::zero;
	math::vec up = math::vec::zero;
	math::vec front = math::vec::zero;

	// Camera frustum
	math::Frustum frustum;

	// Field of View
	bool isPerspective = true;
	float h_fov_rads = 0.0f;
	float v_fov_rads = 30.0f;
	float h_fov_degrees = 0.0f;
	float v_fov_degrees = 0.0f;

	// Aspect Ratio
	float width = 0.f;
	float height = 0.f;

	// View & Projection
	bool need_recalculation = true;
	math::float4x4 global_view = math::float4x4::identity;
	math::float4x4 global_projection = math::float4x4::identity;

	// Debug Drawing
	bool draw_frustum = true;
	bool override_cull = false;

	// Skybox
	bool usingSkybox = true;
	const char* skyboxMD5 = nullptr;
};

#endif // !__RE_CCOMPAMERA_H__