#ifndef __NEW_RE_COMPTRANSFORM_H__
#define __NEW_RE_COMPTRANSFORM_H__

#include "RE_Component.h"
#include "Globals.h"
#include "MathGeoLib\include\MathGeoLib.h"

class RE_CompTransform: public RE_Component
{
public:
	RE_CompTransform();
	~RE_CompTransform();

	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const UID parent) override;

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