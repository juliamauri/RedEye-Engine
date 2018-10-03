#ifndef __RE_COMPTRANSFORM_H__
#define __RE_COMPTRANSFORM_H__

#include "RE_Component.h"
#include "MathGeoLib\include\MathGeoLib.h"

class RE_CompTransform : public RE_Component
{
public:
	RE_CompTransform(RE_GameObject* go = nullptr, math::vec position = math::vec::zero);
	~RE_CompTransform();

	void Update();

	void SetPos(const math::vec position);
	void SetRot(const math::vec euler);
	void SetRot(const math::Quat quat);
	void SetScale(const math::vec scale);

	math::vec GetPosition()const;
	math::vec GetRotXYZ()const;
	math::Quat GetRot()const;
	math::vec GetScale()const;

	math::float4x4 GetGlobalMatrix()const;

	void DrawProperties();

	void CalcGlobalTransform(bool call_tranf_modified = true);

protected:

	math::vec pos = math::vec::zero;
	math::vec rot_eul = math::vec::zero;
	math::Quat rot_quat = math::Quat::identity;
	math::float3 scale = math::vec::one;

	math::float4x4 local_transform = math::float4x4::identity;
	math::float4x4 global_transform = math::float4x4::identity;

private:

	bool transform_modified = false;
};

#endif // !__RE_COMPTRANSFORM_H__