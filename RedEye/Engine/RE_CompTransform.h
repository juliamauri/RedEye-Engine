#ifndef __NEW_RE_COMPTRANSFORM_H__
#define __NEW_RE_COMPTRANSFORM_H__

#include "RE_Component.h"
#include <MGL/MathGeoLib.h>

class RE_CompTransform: public RE_Component
{
public:
	RE_CompTransform();
	~RE_CompTransform() final = default;

	COMP_UID PoolSetUp(class GameObjectsPool* pool, const GO_UID parent, bool report_parent = false) final;
	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent) final;

	void Update();
	bool CheckUpdate();

	void DrawProperties();

	void OnTransformModified();

	math::float4x4 UpdateGlobalMatrixFromParent(math::float4x4 parent);

	// Setters
	void SetRotation(math::Quat rotation);
	void SetRotation(math::vec rotation);
	void SetRotation(math::float3x3 rotation);
	void SetScale(math::vec scale);
	void SetPosition(math::vec position);
	void SetGlobalPosition(math::vec global_position);

	// Movement Controls
	void LocalPan(float rad_dx, float rad_dy, float rad_dz = 0.f);
	void LocalMove(Direction dir, float speed);
	void Orbit(float rad_dx, float rad_dy, const math::vec center);
	void Focus(const math::vec center, float v_fov_rads, float h_fov_rads, float radius = 1.f, float min_dist = 3.0f);

	// Getters
	math::vec GetRight();
	math::vec GetLeft();
	math::vec GetUp();
	math::vec GetDown();
	math::vec GetFront();
	math::vec GetBack();
	math::Quat GetLocalQuaternionRotation() const;
	math::vec GetLocalEulerRotation() const;
	math::vec GetLocalScale() const;
	math::vec GetLocalPosition() const;
	math::vec GetGlobalPosition();
	math::float4x4 GetLocalMatrix();
	math::float4x4 GetGlobalMatrix();
	const float* GetGlobalMatrixPtr();

private:

	void CalcGlobalTransform();

private:

	bool needed_update_transform = false;

	// Position
	math::vec pos = math::vec::zero;

	// Rotation
	math::vec rot_eul = math::vec::zero;
	math::Quat rot_quat = math::Quat::identity;
	math::float3x3 rot_mat = math::float3x3::identity;

	// Scale
	math::ScaleOp scale;

	// Matrix
	math::float4x4 model_local = math::float4x4::identity;
	math::float4x4 model_global = math::float4x4::identity;
};

#endif // !__NEW_RE_COMPTRANSFORM_H__