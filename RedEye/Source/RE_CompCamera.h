#ifndef __RE_COMPCAMERA_H__
#define __RE_COMPCAMERA_H__

#include "RE_Component.h"
#include "RE_Math.h"
#include "Globals.h"

class RE_CompTransform;

class RE_CompCamera : public RE_Component
{
public:
	RE_CompCamera(
		RE_GameObject* go = nullptr,
		bool toPerspective = true,
		float near_plane = 1.0f,
		float far_plane = 5000.0f,
		float v_fov = 30.f,
		bool draw_frustum = true);

	RE_CompCamera(
		const RE_CompCamera& cmpCamera,
		RE_GameObject* go);

	~RE_CompCamera();
	
	void Update() override;
	void DrawProperties() override;
	void DrawFrustum() const;

	RE_CompTransform* GetTransform() const;
	void OnTransformModified() override;

	math::Frustum GetFrustumLocal() const;
	math::Frustum GetFrustumGlobal() const;

	void SetPlanesDistance(float near_plane, float far_plane);
	void SetFOV(float vertical_fov_degrees);
	void ResetAspectRatio(float width, float height);
	void SetPerspective();
	void SetOrthographic();
	void SwapCameraType();

	float GetVFOVDegrees() const;
	float GetHFOVDegrees() const;

	math::float4x4 GetView() const;
	float* GetViewPtr() const;
	
	math::float4x4 GetProjection() const;
	float* GetProjectionPtr() const;

	// Camera Controls
	void LocalRotate(float dx, float dy);
	void LocalMove(Dir dir, float speed);
	void Orbit(float dx, float dy, RE_GameObject* focus);
	void Focus(RE_GameObject* focus, float min_dist = 3.0f);

	// local camera Axis
	math::vec GetRight() const;
	math::vec GetUp() const;
	math::vec GetFront() const;

	void Serialize(JSONNode* node, rapidjson::Value* comp_array) override;

private:
	
	void RecalculateMatrixes();

private:

	// Transform
	RE_CompTransform* transform = nullptr;

	// Axis
	math::vec right = math::vec::zero;
	math::vec up = math::vec::zero;
	math::vec front = math::vec::zero;

	// Focus
	math::vec focus_global_pos = math::vec::zero;

	// Camera frustum
	math::Frustum frustum;

	float near_plane = 0.0f;
	float far_plane = 0.0f;

	bool isPerspective = true;
	float h_fov_rads = 0.0f;
	float v_fov_rads = 30.0f;
	float h_fov_degrees = 0.0f;
	float v_fov_degrees = 0.0f;

	// View & Projection
	bool need_recalculation = false;
	math::float4x4 calculated_view;
	math::float4x4 calculated_projection;

	// Debug Drawing
	bool draw_frustum = true;
};

#endif // !__RE_CCOMPAMERA_H__