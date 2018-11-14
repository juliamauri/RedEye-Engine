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
		float far_plane = 100.0f);
	
	void Update() override;
	void Draw() override;
	void DrawProperties() override;

	RE_CompTransform* GetTransform() const;
	void OnTransformModified() override;

	void SetEulerAngle(float pitch, float yaw);
	void SetPlanesDistance(float near_plane, float far_plane);
	void SwapCameraType();

	// Call this function if window size changed.
	void ResetFov();

	math::float4x4 GetView() const;
	float* GetViewPtr() const;
	
	math::float4x4 GetProjection() const;
	float* GetProjectionPtr() const;

	void RotateWithMouse(float xoffset, float yoffset, bool constrainPitch = true);
	void MouseWheelZoom(float yoffset);

private:
	
	void RecalculateMatrixes();

private:

	// Transform
	RE_CompTransform* transform = nullptr;

	// Camera frustum
	math::Frustum frustum;
	bool isPerspective = true;
	float yaw = 0.0f;
	float pitch = 0.0f;

	// View & Projection
	bool need_recalculation = false;
	math::float4x4 calculated_view;
	math::float4x4 calculated_projection;

	// Debug Drawing
	bool draw_frustum = true;
};

#endif // !__RE_CCOMPAMERA_H__