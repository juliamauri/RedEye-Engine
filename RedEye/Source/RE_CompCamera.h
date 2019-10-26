#ifndef __RE_COMPCAMERA_H__
#define __RE_COMPCAMERA_H__

#include "RE_Component.h"
#include "RE_Math.h"
#include "Globals.h"

class RE_CompTransform;

class RE_CompCamera : public RE_Component
{
public:
	//Camera initialize world origin at (0, 0, 0) and planes distance 0.1f near and 100.0f far
	//@param toPerspective -> true = Prespective | false = Orthographic
	//@param near and @far -> distances of planes from camera position
	RE_CompCamera(
		RE_GameObject* go = nullptr,
		bool toPerspective = true, 
		float near_plane = 0.1f,
		float far_plane = 10000.0f);

	RE_CompCamera(
		RE_GameObject* go,
		bool toPerspective,
		float near_plane,
		float far_plane,
		float h_fov_rads, float v_fov_rads, 
		float h_fov_degrees, float v_fov_degrees, 
		math::vec position, math::vec rotation, math::vec scale);

	RE_CompCamera(
		const RE_CompCamera& cmpCamera,
		RE_GameObject* go);

	~RE_CompCamera();
	
	void Update() override;
	void Draw() override;
	void DrawProperties() override;

	RE_CompTransform* GetTransform() const;
	void OnTransformModified() override;

	void SetPlanesDistance(float near_plane, float far_plane);
	void SwapCameraType();

	// FOV
	float GetVFOVDegrees() const;
	float GetHFOVDegrees() const;
	void SetVerticalFOV(float vertical_fov_degrees);

	// Call this function if window size changed.
	void ResetAspectRatio();

	math::float4x4 GetView() const;
	float* GetViewPtr() const;
	
	math::float4x4 GetProjection() const;
	float* GetProjectionPtr() const;

	math::Frustum GetFrustum() const;

	// Camera Controls
	void LocalRotate(float dx, float dy);
	void LocalMove(Dir dir, float speed);
	void Orbit(float dx, float dy, RE_GameObject* focus);
	void Focus(RE_GameObject* focus, float min_dist);

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

	// Values from frustum
	bool isPerspective = true;

	float near_plane = 0.0f;
	float far_plane = 0.0f;

	float h_fov_rads = 0.0f;
	float v_fov_rads = 0.0f;

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