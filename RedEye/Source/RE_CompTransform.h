#ifndef __RE_COMPTRANSFORM_H__
#define __RE_COMPTRANSFORM_H__

#include "RE_Component.h"
#include "Globals.h"
#include "MathGeoLib\include\MathGeoLib.h"

class RE_CompTransform : public RE_Component
{
public:
	RE_CompTransform(RE_GameObject* go = nullptr);
	~RE_CompTransform();

	void Update();

	void SetPos(math::vec position);
	void SetLocalRot(math::vec euler);
	void SetLocalRot(math::Quat quat);
	void SetScale(math::vec scale);

	void LocalRotate(math::vec axis, float angle);
	void GlobalRotate(math::vec axis, float angle);

	math::vec	GetGlobalPosition() const;
	math::Quat	GetGlobalRot() const;
	math::vec	GetGlobalScale() const;

	math::vec	GetGlobalRight() const;
	math::vec	GetGlobalUp() const;
	math::vec	GetGlobalForward() const;

	math::vec	GetLocalPosition() const;
	math::vec	GetLocalRotXYZ() const;
	math::Quat	GetLocalRot() const;
	math::vec	GetLocalScale() const;

	math::float4x4 GetLocalMatrix() const;
	math::float4x4 GetGlobalMatrix() const;
	math::float4x4 GetGlobalMatInvTrans() const;

	void DrawProperties();
	void Reset();
	bool HasChanged() const;

	void LocalLookAt(math::vec& target_pos);
	void LocalMove(Dir dir, float speed);
	void Orbit(float xoffset, float yoffset, math::vec& target);

	//void GlobalLookAt(math::vec& target_pos);
	//void GlobalMove(Dir dir, float speed);

private:

	void CalcGlobalTransform(bool call_tranf_modified = true);

private:

	// Position
	math::vec pos = math::vec::zero;

	// Rotation
	math::vec rot_eul = math::vec::zero;
	math::Quat rot_quat = math::Quat::identity;
	
	// Scale
	math::vec scale = math::vec::one;

	// Axis
	math::vec right = math::vec::zero;
	math::vec up = math::vec::zero;
	math::vec front = math::vec::zero;

	//math::float4x4 local_transform = math::float4x4::identity;
	//math::float4x4 global_transform = math::float4x4::identity;

	bool transform_modified = false;
	bool just_calculated_global = false;
};

#endif // !__RE_COMPTRANSFORM_H__