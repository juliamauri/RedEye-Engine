#ifndef __NEW_RE_COMPTRANSFORM_H__
#define __NEW_RE_COMPTRANSFORM_H__

#include "RE_Component.h"
#include "Globals.h"
#include "MathGeoLib\include\MathGeoLib.h"

class RE_CompTransform: public RE_Component
{
public:
	RE_CompTransform(RE_GameObject* go = nullptr);
	~RE_CompTransform();

	void Update();

	math::float4x4 GetMatrixModel();
	float* GetShaderModel();
	
	void SetRotation(math::Quat rotation);
	void SetRotation(math::vec rotation);
	void SetScale(math::vec scale);
	void SetPosition(math::vec position);

	math::Quat GetQuaternionRotation();
	math::vec GetEulerRotation();
	math::vec GetScale();
	math::vec GetPosition();
	math::vec GetGlobalPosition();

	void LocalMove(Dir dir, float speed);

	void DrawProperties();

	bool HasChanged();

private:

	void CalcGlobalTransform();

	bool needed_update_transform = false;
	bool has_changed = false;

	// Position
	math::vec pos = math::vec::zero;

	// Rotation
	math::vec rot_eul = math::vec::zero;
	math::Quat rot_quat = math::Quat::identity;
	bool using_euler = false;

	// Scale
	math::ScaleOp scale;

	// Axis
	math::vec right = math::vec::zero;
	math::vec up = math::vec::zero;
	math::vec front = math::vec::zero;

	// Matrix
	math::float4x4 model_local = math::float4x4::identity;
	math::float4x4 model_global = math::float4x4::identity;

	//Separate from parent
	bool useParent = true;
};

#endif // !__NEW_RE_COMPTRANSFORM_H__