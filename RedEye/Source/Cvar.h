#ifndef __CVAR__
#define __CVAR__

#include <string>

#include "MathGeoLib/include/Math/float2.h"
#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float4.h"
#include "MathGeoLib/include/Math/float3x3.h"
#include "MathGeoLib/include/Math/float4x4.h"

class RE_GameObject;

class Cvar // Global Value Container
{
public:
	Cvar();
	Cvar(const Cvar& copy);
	Cvar(bool bool_v);
	Cvar(int int_v);
	Cvar(unsigned int uint_v);
	Cvar(long long int int64_v);
	Cvar(unsigned long long int uint64_v);
	Cvar(double double_v);
	Cvar(float float_v);
	Cvar(const char* char_p_v);
	Cvar(RE_GameObject* go_v);

public:
	enum VAR_TYPE : unsigned int
	{
		UNDEFINED,
		BOOL,
		BOOL2,
		BOOL3,
		BOOL4,
		INT,
		INT2,
		INT3,
		INT4,
		UINT,
		INT64,
		UINT64,
		DOUBLE,
		FLOAT,
		FLOAT2,
		FLOAT3,
		FLOAT4,
		MAT2,
		MAT3,
		MAT4,
		CHAR_P,
		GAMEOBJECT,
		SAMPLER
	};

protected:
	VAR_TYPE type;

	union VAR_data
	{
		bool bool_v;
		bool bool2_v[2];
		bool bool3_v[3];
		bool bool4_v[4];
		int int_v;
		int int2_v[2];
		int int3_v[3];
		int int4_v[4];
		unsigned int uint_v;
		long long int int64_v;
		unsigned long long int uint64_v;
		double double_v;
		float float_v;
		math::float2 float2_v;
		math::float3 float3_v;
		math::float4 float4_v;
		const char* char_p_v;
		RE_GameObject* go_v;
		math::float3x3 mat3_v;
		math::float4x4 mat4_v;
		VAR_data() { }
		~VAR_data() { }
	} value;

public:
	virtual bool SetValue(bool bool_v, bool force_type = false);
	virtual bool SetValue(int int_v, bool force_type = false);
	virtual bool SetValue(unsigned int uint_v, bool force_type = false);
	virtual bool SetValue(long long int int64_v, bool force_type = false);
	virtual bool SetValue(unsigned long long int uint64_v, bool force_type = false);
	virtual bool SetValue(double double_v, bool force_type = false);
	virtual bool SetValue(float float_v, bool force_type = false);
	virtual bool SetValue(const char* char_p_v, bool force_type = false);
	virtual bool SetValue(RE_GameObject* go_v, bool force_type = false);

	VAR_TYPE				GetType() const;
	bool					AsBool() const;
	bool*					AsBool2();
	bool*					AsBool3();
	bool*					AsBool4();
	int						AsInt() const;
	int*					AsInt2();
	int*					AsInt3();
	int*					AsInt4();
	unsigned int			AsUInt() const;
	long long int			AsInt64() const;
	unsigned long long int	AsUInt64() const;
	double					AsDouble() const;
	float					AsFloat() const;
	math::float2			AsFloat2() const;
	math::float3			AsFloat3() const;
	math::float4			AsFloat4() const;
	math::float3x3			AsMat3()const;
	math::float4x4			AsMat4()const;
	float*					AsFloatPointer();
	const char*				AsCharP() const;
	RE_GameObject*			AsGO() const;
};

class ShaderCvar : public Cvar
{
public: 
	ShaderCvar();
	ShaderCvar(const ShaderCvar& copy);
	ShaderCvar(bool bool_v);
	ShaderCvar(int int_v, bool sampler = false);
	ShaderCvar(float float_v);
	ShaderCvar(bool boola_v[], unsigned int count);
	ShaderCvar(int inta_v[], unsigned int count);
	ShaderCvar(math::float2 float2_v);
	ShaderCvar(math::float3 float3_v);
	ShaderCvar(math::float4 float4_v, bool mat2 = false);
	ShaderCvar(math::float3x3 mat3_v);
	ShaderCvar(math::float4x4 mat4_v);

	bool SetValue(bool bool_v, bool force_type = false) override;
	bool SetValue(bool boola_v[], unsigned int count, bool force_type = false);
	bool SetValue(int int_v, bool force_type = false) override;
	bool SetValue(int inta_v[], unsigned int count, bool force_type = false);
	bool SetValue(float float_v, bool force_type = false) override;
	bool SetValue(math::float2 float2_v, bool force_type = false);
	bool SetValue(math::float3 float3_v, bool force_type = false);
	bool SetValue(math::float4 float4_v, bool mat2 = false, bool force_type = false);
	bool SetValue(math::float3x3 mat3_v, bool force_type = false);
	bool SetValue(math::float4x4 mat4_v, bool force_type = false);
	bool SetSampler(const char* res_ptr, bool force_type = false);

	bool DrawPropieties(bool isInMemory);

	std::string name;
	int location = 0;
	bool custom = true;
};

/*class DoubleCvar : public Cvar
{
public:
	DoubleCvar(bool bool_v);
	DoubleCvar(int int_v);
	DoubleCvar(unsigned int uint_v);
	DoubleCvar(long long int int64_v);
	DoubleCvar(unsigned long long int uint64_v);
	DoubleCvar(double double_v);
	DoubleCvar(float float_v);
	DoubleCvar(const char* char_p_v);

	bool ValueHasChanged() const;

	bool SetValue(bool bool_v, bool force_type = false) override;
	bool SetValue(int int_v, bool force_type = false) override;
	bool SetValue(unsigned int uint_v, bool force_type = false) override;
	bool SetValue(long long int int64_v, bool force_type = false) override;
	bool SetValue(unsigned long long int uint64_v, bool force_type = false) override;
	bool SetValue(double double_v, bool force_type = false) override;
	bool SetValue(float float_v, bool force_type = false) override;
	bool SetValue(const char* char_p_v, bool force_type = false) override;

private:
	VAR_data original_value;
};
*/

#endif // !__CVAR__