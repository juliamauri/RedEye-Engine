#ifndef __CVAR__
#define __CVAR__

#include <EASTL/string.h>

#include <MGL/Math/float2.h>
#include <MGL/Math/float3.h>
#include <MGL/Math/float4.h>
#include <MGL/Math/float3x3.h>
#include <MGL/Math/float4x4.h>

class RE_GameObject;

class RE_Cvar // Global Value Container
{
public:

	RE_Cvar();
	RE_Cvar(const RE_Cvar& copy);
	RE_Cvar(bool bool_v);
	RE_Cvar(int int_v);
	RE_Cvar(unsigned int uint_v);
	RE_Cvar(long long int int64_v);
	RE_Cvar(unsigned long long int uint64_v);
	RE_Cvar(double double_v);
	RE_Cvar(float float_v);
	RE_Cvar(const char* char_p_v);
	RE_Cvar(eastl::string string_v);
	RE_Cvar(RE_GameObject* go_v);
	RE_Cvar(const math::float4x4 mat4_v);

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
		STRING,

		GAMEOBJECT,
		SAMPLER
	};

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

		math::float3x3 mat3_v;
		math::float4x4 mat4_v;

		const char* char_p_v;
		eastl::string string_v = "";

		RE_GameObject* go_v;

		VAR_data() {}
		~VAR_data() {}
	};

protected:

	VAR_TYPE type;
	VAR_data value;

public:

	virtual bool SetValue(bool bool_v, bool force_type = false);
	virtual bool SetValue(int int_v, bool force_type = false);
	virtual bool SetValue(unsigned int uint_v, bool force_type = false);
	virtual bool SetValue(long long int int64_v, bool force_type = false);
	virtual bool SetValue(unsigned long long int uint64_v, bool force_type = false);
	virtual bool SetValue(double double_v, bool force_type = false);
	virtual bool SetValue(float float_v, bool force_type = false);
	virtual bool SetValue(const char* char_p_v, bool force_type = false);
	virtual bool SetValue(eastl::string string_v, bool force_type = false);
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
	math::float3x3			AsMat3() const;
	math::float4x4			AsMat4() const;
	float*					AsFloatPointer();
	const float*			AsFloatPointer() const;
	const char*				AsCharP() const;
	RE_GameObject*			AsGO() const;

	bool operator==(const RE_Cvar& other) const;
};

class RE_Shader_Cvar : public RE_Cvar
{
public: 
	RE_Shader_Cvar();
	RE_Shader_Cvar(const RE_Shader_Cvar& copy);
	RE_Shader_Cvar(const bool bool_v);
	RE_Shader_Cvar(const int int_v, bool sampler = false);
	RE_Shader_Cvar(const float float_v);
	RE_Shader_Cvar(const bool boola_v[], unsigned int count);
	RE_Shader_Cvar(const int inta_v[], unsigned int count);
	RE_Shader_Cvar(const math::float2 float2_v);
	RE_Shader_Cvar(const math::float3 float3_v);
	RE_Shader_Cvar(const math::float4 float4_v, bool mat2 = false);
	RE_Shader_Cvar(const math::float3x3 mat3_v);
	RE_Shader_Cvar(const math::float4x4 mat4_v);

	RE_Shader_Cvar operator=(const RE_Shader_Cvar& cpy);

	bool SetValue(const RE_Shader_Cvar& copyValue, bool force_type = false);
	bool SetValue(const bool bool_v, bool force_type = false) override;
	bool SetValue(const bool boola_v[], unsigned int count, bool force_type = false);
	bool SetValue(const int int_v, bool force_type = false) override;
	bool SetValue(const int inta_v[], unsigned int count, bool force_type = false);
	bool SetValue(const float float_v, bool force_type = false) override;
	bool SetValue(const math::float2 float2_v, bool force_type = false);
	bool SetValue(const math::float3 float3_v, bool force_type = false);
	bool SetValue(const math::float4 float4_v, bool mat2 = false, bool force_type = false);
	bool SetValue(const math::float3x3 mat3_v, bool force_type = false);
	bool SetValue(const math::float4x4 mat4_v, bool force_type = false);
	bool SetSampler(const char* res_ptr, bool force_type = false);

	bool DrawPropieties(bool isInMemory);

	eastl::string name;
	int location = 0, locationDeferred = 0;
	bool custom = true;
};

/*class RE_Double_Cvar : public RE_Cvar
{
public:
	RE_Double_Cvar(bool bool_v);
	RE_Double_Cvar(int int_v);
	RE_Double_Cvar(unsigned int uint_v);
	RE_Double_Cvar(long long int int64_v);
	RE_Double_Cvar(unsigned long long int uint64_v);
	RE_Double_Cvar(double double_v);
	RE_Double_Cvar(float float_v);
	RE_Double_Cvar(const char* char_p_v);

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