#ifndef __SIMPLE_CAMERA_H__
#define __SIMPLE_CAMERA_H__

#include <MGL/MathGeoLib.h>
#include "RE_DataTypes.h"
#include "RE_Serializable.h"

class RE_Camera : public RE_Serializable
{
public:

	enum class AspectRatio : uint
	{
		Fit_Window = 0,
		Square_1x1,
		TraditionalTV_4x3,
		Movietone_16x9,
		Personalized
	};

private:

	math::Frustum frustum;

	// Aspect Ratio
	AspectRatio target_ar = AspectRatio::Fit_Window;
	float width = 0.f;
	float height = 0.f;

public:

	RE_Camera() = default;
	~RE_Camera() = default;

	void SetupFrustum(
		float n_plane = 1.0f,
		float f_plane = 5000.0f,
		RE_Camera::AspectRatio ar = RE_Camera::AspectRatio::Fit_Window,
		float v_fov = 0.523599f);

	void SetFrame(
		const math::vec& pos,
		const math::vec& front,
		const math::vec& up);

	// Draws
	bool DrawProperties();
	void DrawAsEditorProperties();
	void DrawFrustum() const;

	// Camera Controls
	void Move(Direction dir, float speed);
	void Pan(float rad_dx, float rad_dy, float rad_dz = 0.f);
	void Orbit(float rad_dx, float rad_dy, math::vec center);
	void Focus(math::vec center, float radius = 1.f, float min_dist = 3.0f);
	void Focus(math::AABB box, float min_dist = 3.0f);

	// Setters
	void SetPlanesDistance(float near_plane, float far_plane) { frustum.SetViewPlaneDistances(near_plane, far_plane); }
	void SetFOVRads(float vertical_fov_rads);
	void SetFOVDegrees(float vertical_fov_degrees);
	void SetAspectRatio(AspectRatio aspect_ratio, bool apply_changes = true);
	void SetBounds(float width, float height, bool apply_changes = true);
	void SetPerspective(float default_vfov_degrees = 50.f);
	void SetOrthographic();
	void SwapCameraType();

	// Getters - Frustum
	const math::Frustum& GetFrustum() const { return frustum; }
	bool IsPerspective() const { return frustum.Type() == PerspectiveFrustum; }
	bool IsOrthographic() const { return frustum.Type() == OrthographicFrustum; }
	float GetNearPlane() const { return frustum.NearPlaneDistance(); }
	float GetFarPlane() const { return frustum.FarPlaneDistance(); }
	float GetVFOVRads() const { return IsPerspective() ? frustum.VerticalFov() : -1.f; }
	float GetHFOVRads() const { return IsPerspective() ? frustum.HorizontalFov() : -1.f; }
	AspectRatio GetAspectRatio() const { return target_ar; }

	// Getters - Viewport
	void GetTargetViewPort(math::float4& viewPort) const;
	float GetTargetWidth() const { return width; }
	float GetTargetHeight() const { return height; }
	void GetTargetWidthHeight(int& _width, int& _height) const
	{
		_width = static_cast<int>(width);
		_height = static_cast<int>(height);
	}
	void GetTargetWidthHeight(float& _width, float& _height) const
	{
		_width = width;
		_height = height;
	}

	// Getters - Matrixes
	math::float4x4 GetView() const { return math::float4x4(frustum.ViewMatrix()).Transposed(); }
	math::float4x4 GetProjection() const { return frustum.ProjectionMatrix().Transposed(); }
	math::float4x4 GetViewProjMatrix() const { return frustum.ViewProjMatrix().Transposed(); }

	// Serialization
	void JsonSerialize(RE_Json* node) const final;
	void JsonDeserialize(RE_Json* node) final;

	size_t GetBinarySize() const final;
	void BinarySerialize(char*& cursor) const final;
	void BinaryDeserialize(char*& cursor) final;
};

#endif // !__SIMPLE_CAMERA_H__