#include "RE_Cvar.h"

#include "Application.h"
#include "RE_ResourceManager.h"
#include "RE_GameObject.h"

#include <ImGui/imgui.h>
#include <EASTL/vector.h>

#pragma region Cvar

RE_Cvar::RE_Cvar() : type(Type::UNDEFINED) { value.int_v = 0; }

RE_Cvar::RE_Cvar(const RE_Cvar & copy) : type(copy.type)
{
	switch (copy.type)
	{
	case Type::BOOL: value.bool_v = copy.value.bool_v; break;
	case Type::BOOL2: memcpy(value.bool2_v,copy.value.bool2_v, sizeof(bool) *2); break;
	case Type::BOOL3: memcpy(value.bool3_v, copy.value.bool3_v, sizeof(bool) * 3); break;
	case Type::BOOL4: memcpy(value.bool4_v, copy.value.bool4_v, sizeof(bool) * 4); break;
	case Type::SAMPLER:
	case Type::INT: value.int_v = copy.value.int_v; break;
	case Type::INT2: memcpy(value.int2_v, copy.value.int2_v, sizeof(int) * 2); break;
	case Type::INT3: memcpy(value.int3_v, copy.value.int3_v, sizeof(int) * 3); break;
	case Type::INT4: memcpy(value.int4_v, copy.value.int4_v, sizeof(int) * 4); break;
	case Type::UINT: value.uint_v = copy.value.uint_v; break;
	case Type::INT64: value.int64_v = copy.value.int64_v; break;
	case Type::UINT64: value.uint64_v = copy.value.uint64_v; break;
	case Type::DOUBLE: value.double_v = copy.value.double_v; break;
	case Type::FLOAT: value.float_v = copy.value.float_v; break;
	case Type::FLOAT2: value.float2_v = copy.value.float2_v; break;
	case Type::FLOAT3: value.float3_v = copy.value.float3_v; break;
	case Type::MAT2:
	case Type::FLOAT4: value.float4_v = copy.value.float4_v; break;
	case Type::CHAR_P: value.char_p_v = copy.value.char_p_v; break;
	case Type::STRING: value.string_v = copy.value.string_v; break;
	case Type::GAMEOBJECT: value.go_v = copy.value.go_v; break;
	case Type::MAT3: value.mat3_v = copy.value.mat3_v; break;
	case Type::MAT4: value.mat4_v = copy.value.mat4_v; break;
	}
}

RE_Cvar::RE_Cvar(bool bool_v) : type(Type::BOOL) { value.bool_v = bool_v; }
RE_Cvar::RE_Cvar(int int_v) : type(Type::INT) { value.int_v = int_v; }
RE_Cvar::RE_Cvar(unsigned int uint_v) : type(Type::UINT) { value.uint_v = uint_v; }
RE_Cvar::RE_Cvar(long long int int64_v) : type(Type::INT64) { value.int64_v = int64_v; }
RE_Cvar::RE_Cvar(unsigned long long int uint64_v) : type(Type::UINT64) { value.uint64_v = uint64_v; }
RE_Cvar::RE_Cvar(double double_v) : type(Type::DOUBLE) { value.double_v = double_v; }
RE_Cvar::RE_Cvar(float float_v) : type(Type::FLOAT) { value.float_v = float_v; }
RE_Cvar::RE_Cvar(const char * char_p_v) : type(Type::CHAR_P) { value.char_p_v = char_p_v; }
RE_Cvar::RE_Cvar(eastl::string string_v) : type(Type::STRING) { value.string_v = string_v; }
RE_Cvar::RE_Cvar(RE_GameObject * go_v) : type(Type::GAMEOBJECT) { value.go_v = go_v; }
RE_Cvar::RE_Cvar(const math::float4x4 mat4_v) : type(Type::MAT4) { value.mat4_v = mat4_v; }

bool RE_Cvar::SetValue(bool bool_v, bool force_type)
{
	bool ret = false;
	if (force_type) type = Type::BOOL;
	if (ret = (type == Type::BOOL)) value.bool_v = bool_v;
	return ret;
}

bool RE_Cvar::SetValue(int int_v, bool force_type)
{
	bool ret = false;
	if (force_type) type = Type::INT;
	if (ret = (type == Type::INT)) value.int_v = int_v;
	return ret;
}

bool RE_Cvar::SetValue(unsigned int uint_v, bool force_type)
{
	bool ret = false;
	if (force_type) type = Type::UINT;
	if (ret = (type == Type::UINT)) value.uint_v = uint_v;
	return ret;
}

bool RE_Cvar::SetValue(long long int int64_v, bool force_type)
{
	bool ret = false;
	if (force_type) type = Type::INT64;
	if (ret = (type == Type::INT64)) value.int64_v = int64_v;
	return ret;
}

bool RE_Cvar::SetValue(unsigned long long int uint64_v, bool force_type)
{
	bool ret = false;
	if (force_type) type = Type::UINT64;
	if (ret = (type == Type::UINT64)) value.uint64_v = uint64_v;
	return ret;
}

bool RE_Cvar::SetValue(double double_v, bool force_type)
{
	bool ret = false;
	if (force_type) type = Type::DOUBLE;
	if (ret = (type == Type::DOUBLE)) value.double_v = double_v;
	return ret;
}

bool RE_Cvar::SetValue(float float_v, bool force_type)
{
	bool ret = false;
	if (force_type) type = Type::FLOAT;
	if (ret = (type == Type::FLOAT)) value.float_v = float_v;
	return ret;
}

bool RE_Cvar::SetValue(const char * char_p_v, bool force_type)
{
	bool ret = false;
	if (force_type) type = Type::CHAR_P;
	if (ret = (type == Type::CHAR_P)) value.char_p_v = char_p_v;
	return ret;
}

bool RE_Cvar::SetValue(eastl::string string_v, bool force_type)
{
	bool ret = false;
	if (force_type) type = Type::STRING;
	if (ret = (type == Type::STRING)) value.string_v = string_v;
	return ret;
}

bool RE_Cvar::SetValue(RE_GameObject * go_v, bool force_type)
{
	bool ret = false;
	if (force_type) type = Type::GAMEOBJECT;
	if (ret = (type == Type::GAMEOBJECT)) value.go_v = go_v;
	return ret;
}

RE_Cvar::Type RE_Cvar::GetType() const { return type; }
bool RE_Cvar::AsBool() const { return value.bool_v; }
bool* RE_Cvar::AsBool2() { return value.bool2_v; }
bool* RE_Cvar::AsBool3() { return value.bool3_v; }
bool* RE_Cvar::AsBool4() { return value.bool4_v; }
const bool* RE_Cvar::AsBool2() const { return value.bool2_v; }
const bool* RE_Cvar::AsBool3() const { return value.bool3_v; }
const bool* RE_Cvar::AsBool4() const { return value.bool4_v; }
int RE_Cvar::AsInt() const { return value.int_v; }
int* RE_Cvar::AsInt2() { return value.int2_v; }
int* RE_Cvar::AsInt3() { return value.int3_v; }
int* RE_Cvar::AsInt4() { return value.int4_v; }
const int* RE_Cvar::AsInt2() const { return value.int2_v; }
const int* RE_Cvar::AsInt3() const { return value.int3_v; }
const int* RE_Cvar::AsInt4() const { return value.int4_v; }
uint RE_Cvar::AsUInt() const { return value.uint_v; }
long long RE_Cvar::AsInt64() const { return value.int64_v; }
ulonglong RE_Cvar::AsUInt64() const { return value.uint64_v; }
double RE_Cvar::AsDouble() const { return value.double_v; }
float RE_Cvar::AsFloat() const { return value.float_v; }
math::float2 RE_Cvar::AsFloat2() const { return value.float2_v; }
math::float3 RE_Cvar::AsFloat3() const { return value.float3_v; }
math::float4 RE_Cvar::AsFloat4() const { return value.float4_v; }
math::float3x3 RE_Cvar::AsMat3() const { return value.mat3_v; }
math::float4x4 RE_Cvar::AsMat4() const { return value.mat4_v; }

float* RE_Cvar::AsFloatPointer()
{ 
	float* ret = nullptr;

	switch (type)
	{
	case Type::FLOAT: ret = &value.float_v; break;
	case Type::FLOAT2: ret = value.float2_v.ptr(); break;
	case Type::FLOAT3: ret = value.float3_v.ptr(); break;
	case Type::FLOAT4:
	case Type::MAT2: ret = value.float4_v.ptr(); break;
	case Type::MAT3: ret = value.mat3_v.ptr(); break;
	case Type::MAT4: ret = value.mat4_v.ptr(); break;
	default: break;
	}

	return ret;
}

const float* RE_Cvar::AsFloatPointer() const
{
	const float* ret = nullptr;

	switch (type)
	{
	case Type::FLOAT: ret = &value.float_v; break;
	case Type::FLOAT2: ret = value.float2_v.ptr(); break;
	case Type::FLOAT3: ret = value.float3_v.ptr(); break;
	case Type::FLOAT4:
	case Type::MAT2: ret = value.float4_v.ptr(); break;
	case Type::MAT3: ret = value.mat3_v.ptr(); break;
	case Type::MAT4: ret = value.mat4_v.ptr(); break;
	default: break;
	}

	return ret;
}

const char * RE_Cvar::AsCharP() const
{
	switch (type)
	{
	case Type::CHAR_P: return value.char_p_v;
	case Type::STRING: return value.string_v.c_str();
	default: break;
	}
	return nullptr;
}

RE_GameObject * RE_Cvar::AsGO() const { return value.go_v; }

bool RE_Cvar::operator==(const RE_Cvar& other) const
{
	if (type != other.type) return false;

	switch (type)
	{
	case Type::UNDEFINED: break;
	case Type::BOOL: return value.bool_v == other.value.bool_v;
	case Type::BOOL2: return
		value.bool2_v[0] == other.value.bool2_v[0] &&
		value.bool2_v[1] == other.value.bool2_v[1];
	case Type::BOOL3: return
		value.bool3_v[0] == other.value.bool3_v[0] &&
		value.bool3_v[1] == other.value.bool3_v[1] &&
		value.bool3_v[2] == other.value.bool3_v[2];
	case Type::BOOL4: return
		value.bool4_v[0] == other.value.bool4_v[0] &&
		value.bool4_v[1] == other.value.bool4_v[1] &&
		value.bool4_v[2] == other.value.bool4_v[2] &&
		value.bool4_v[3] == other.value.bool4_v[3];
	case Type::INT: return value.int_v == other.value.int_v;
	case Type::INT2: return
		value.int2_v[0] == other.value.int2_v[0] &&
		value.int2_v[1] == other.value.int2_v[1];
	case Type::INT3: return
		value.int3_v[0] == other.value.int3_v[0] &&
		value.int3_v[1] == other.value.int3_v[1] &&
		value.int3_v[2] == other.value.int3_v[2];
	case Type::INT4: return
		value.int4_v[0] == other.value.int4_v[0] &&
		value.int4_v[1] == other.value.int4_v[1] &&
		value.int4_v[2] == other.value.int4_v[2] &&
		value.int4_v[3] == other.value.int4_v[3];
	case Type::UINT: return value.uint_v == other.value.uint_v;
	case Type::INT64: return value.int64_v == other.value.int64_v;
	case Type::UINT64: return value.uint64_v == other.value.uint64_v;
	case Type::DOUBLE: return value.double_v == other.value.double_v;
	case Type::FLOAT: return value.float_v == other.value.float_v;
	case Type::FLOAT2: return
		value.float2_v[0] == other.value.float2_v[0] &&
		value.float2_v[1] == other.value.float2_v[1];
	case Type::FLOAT3: return
		value.float3_v[0] == other.value.float3_v[0] &&
		value.float3_v[1] == other.value.float3_v[1] &&
		value.float3_v[2] == other.value.float3_v[2];
	case Type::FLOAT4:
	case Type::MAT2: return
		value.float4_v[0] == other.value.float4_v[0] &&
		value.float4_v[1] == other.value.float4_v[1] &&
		value.float4_v[2] == other.value.float4_v[2] &&
		value.float4_v[3] == other.value.float4_v[3];
	case Type::MAT3:
	{
		bool ret = true;
		for (short i = 0; i < 9; i++)
			ret &= value.mat3_v.At(i % 3, i / 3) == other.value.mat3_v.At(i % 3, i / 3);
		return ret; 
	}
	case Type::MAT4:
	{
		bool ret = true;
		for (short i = 0; i < 16; i++)
			ret &= value.mat4_v.At(i % 4, i / 4) == other.value.mat4_v.At(i % 4, i / 4);
		return ret; 
	}
	case Type::CHAR_P: return eastl::string(value.char_p_v).compare(other.value.char_p_v) == 0;
	case Type::STRING: return value.string_v.compare(other.value.string_v) == 0;
	case Type::GAMEOBJECT: return value.go_v == other.value.go_v;
	case Type::SAMPLER: break;
	default: break;
	}

	return false;
}

#pragma endregion

#pragma region Shader Cvar

RE_Shader_Cvar::RE_Shader_Cvar() : RE_Cvar() {}
RE_Shader_Cvar::RE_Shader_Cvar(const RE_Shader_Cvar& copy) : RE_Cvar(copy), name(copy.name), location(copy.location), locationDeferred(copy.locationDeferred), custom(copy.custom) { }
RE_Shader_Cvar::RE_Shader_Cvar(const bool bool_v) : RE_Cvar(bool_v) {}
RE_Shader_Cvar::RE_Shader_Cvar(const int int_v, bool sampler)
{
	RE_Cvar::type = (sampler) ? Type::SAMPLER : Type::INT;
	RE_Cvar::value.int_v = int_v;
}
RE_Shader_Cvar::RE_Shader_Cvar(float float_v) : RE_Cvar(float_v) {}
RE_Shader_Cvar::RE_Shader_Cvar(const bool boola_v[], unsigned int count)
{
	switch (count)
	{
	case 2: RE_Cvar::type = Type::BOOL2; memcpy(value.bool2_v, boola_v, sizeof(bool) * 2); break;
	case 3: RE_Cvar::type = Type::BOOL3; memcpy(value.bool3_v, boola_v, sizeof(bool) * 3); break;
	case 4: RE_Cvar::type = Type::BOOL4; memcpy(value.bool4_v, boola_v, sizeof(bool) * 4); break;
	default: RE_Cvar::type = Type::BOOL; value.bool_v = boola_v[0]; break;
	}
}

RE_Shader_Cvar::RE_Shader_Cvar(const int inta_v[], unsigned int count)
{
	switch (count)
	{
	case 2: RE_Cvar::type = Type::INT2; memcpy(value.int2_v, inta_v, sizeof(int) * 2); break;
	case 3: RE_Cvar::type = Type::INT3; memcpy(value.int3_v, inta_v, sizeof(int) * 3); break;
	case 4: RE_Cvar::type = Type::INT4; memcpy(value.int4_v, inta_v, sizeof(int) * 4); break;
	default: RE_Cvar::type = Type::INT; value.int_v = inta_v[0]; break;
	}
}

RE_Shader_Cvar::RE_Shader_Cvar(const math::float2 float2_v) {
	RE_Cvar::type = Type::FLOAT2;
	RE_Cvar::value.float2_v = float2_v; }

RE_Shader_Cvar::RE_Shader_Cvar(const math::float3 float3_v) {
	RE_Cvar::type = Type::FLOAT3;
	RE_Cvar::value.float3_v = float3_v; }

RE_Shader_Cvar::RE_Shader_Cvar(const math::float4 float4_v, bool mat2) {
	RE_Cvar::type = (mat2) ? Type::MAT2 : Type::FLOAT4;
	RE_Cvar::value.float4_v = float4_v; }

RE_Shader_Cvar::RE_Shader_Cvar(const math::float3x3 mat3_v) {
	RE_Cvar::type = Type::MAT3;
	RE_Cvar::value.mat3_v = mat3_v; }

RE_Shader_Cvar::RE_Shader_Cvar(const math::float4x4 mat4_v) {
	RE_Cvar::type = Type::MAT4;
	RE_Cvar::value.mat4_v = mat4_v; }

RE_Shader_Cvar RE_Shader_Cvar::operator=(const RE_Shader_Cvar& cpy)
{
	return RE_Shader_Cvar(cpy);
}

bool RE_Shader_Cvar::SetValue(const RE_Shader_Cvar& copyValue, bool force_type)
{
	bool ret = false;
	if (force_type) type = copyValue.type;

	if (ret = (type == copyValue.type))
	{
		switch (type)
		{
		case Type::BOOL: SetValue(copyValue.value.bool_v); break;
		case Type::BOOL2: SetValue(&copyValue.value.bool2_v[0], 2); break;
		case Type::BOOL3: SetValue(copyValue.value.bool3_v, 3); break;
		case Type::BOOL4: SetValue(copyValue.value.bool4_v, 4); break;
		case Type::INT: SetValue(copyValue.value.int_v); break;
		case Type::INT2: SetValue(copyValue.value.int2_v, 2); break;
		case Type::INT3: SetValue(copyValue.value.int3_v, 3); break;
		case Type::INT4: SetValue(copyValue.value.int4_v, 4); break;
		case Type::FLOAT: SetValue(copyValue.value.float_v); break;
		case Type::FLOAT2: SetValue(copyValue.value.float2_v); break;
		case Type::FLOAT3: SetValue(copyValue.value.float3_v); break;
		case Type::FLOAT4: SetValue(copyValue.value.float4_v); break;
		case Type::MAT2: SetValue(copyValue.value.float4_v); break;
		case Type::MAT3: SetValue(copyValue.value.mat3_v); break;
		case Type::MAT4: SetValue(copyValue.value.mat4_v); break;
		case Type::SAMPLER: SetSampler(copyValue.value.char_p_v); break;
		}
	}

	return ret;
}

bool RE_Shader_Cvar::SetValue(const bool bool_v, bool force_type)
{
	bool ret;
	if (force_type) type = Type::BOOL;
	if (ret = (type == Type::BOOL)) value.bool_v =  bool_v;
	return ret;
}

bool RE_Shader_Cvar::SetValue(const bool boola_v[], unsigned int count, bool force_type)
{
	bool ret = false;

	RE_Cvar::Type toCheck;
	switch (count)
	{
	case 2: toCheck = Type::BOOL2; break;
	case 3: toCheck = Type::BOOL3; break;
	case 4: toCheck = Type::BOOL4; break;
	default: toCheck = Type::BOOL; break;
	}

	if (force_type) type = toCheck;

	if (ret = (type == toCheck))
	{
		switch (count)
		{
		case 2: memcpy(value.bool2_v, boola_v, sizeof(bool) * 2); break;
		case 3: memcpy(value.bool3_v, boola_v, sizeof(bool) * 3); break;
		case 4: memcpy(value.bool4_v, boola_v, sizeof(bool) * 4); break;
		default: value.bool_v = boola_v[0]; break;
		}
	}

	return ret;
}

bool RE_Shader_Cvar::SetValue(const int int_v, bool force_type)
{
	bool ret;
	if (force_type) type = Type::INT;
	if (ret = (type == Type::INT)) value.int_v = int_v;
	return ret;
}

bool RE_Shader_Cvar::SetValue(const int inta_v[], unsigned int count, bool force_type)
{
	bool ret;
	RE_Cvar::Type toCheck;
	switch (count)
	{
	case 2: toCheck = Type::INT2; break;
	case 3: toCheck = Type::INT3; break;
	case 4: toCheck = Type::INT4; break;
	default: toCheck = Type::INT; break;
	}

	if (force_type) type = toCheck;

	if (ret = (type == toCheck))
	{
		switch (count)
		{
		case 2: memcpy(value.int2_v, inta_v, sizeof(int) * 2); break;
		case 3: memcpy(value.int3_v, inta_v, sizeof(int) * 3); break;
		case 4: memcpy(value.int4_v, inta_v, sizeof(int) * 4); break;
		default: value.int_v = inta_v[0]; break;
		}
	}

	return ret;
}

bool RE_Shader_Cvar::SetValue(const float float_v, bool force_type)
{
	bool ret;
	if (force_type) type = Type::FLOAT;
	if (ret = (type == Type::FLOAT)) value.float_v = float_v;
	return ret;
}

bool RE_Shader_Cvar::SetValue(const math::float2 float2_v, bool force_type)
{
	bool ret;
	if (force_type) type = Type::FLOAT2;
	if (ret = (type == Type::FLOAT2)) value.float2_v = float2_v;
	return ret;
}

bool RE_Shader_Cvar::SetValue(const math::float3 float3_v, bool force_type)
{
	bool ret;
	if (force_type) type = Type::FLOAT3;
	if (ret = (type == Type::FLOAT3)) value.float3_v = float3_v;
	return ret;
}

bool RE_Shader_Cvar::SetValue(const math::float4 float4_v, bool mat2, bool force_type)
{
	bool ret;
	if (force_type) type = (mat2) ? Type::MAT2 : Type::FLOAT4;
	if (ret = (type == Type::FLOAT4)) value.float4_v = float4_v;
	return ret;
}

bool RE_Shader_Cvar::SetValue(const math::float3x3 mat3_v, bool force_type)
{
	bool ret;
	if (force_type) type = Type::MAT3;
	if (ret = (type == Type::MAT3)) value.mat3_v = mat3_v;
	return ret;
}

bool RE_Shader_Cvar::SetValue(const math::float4x4 mat4_v, bool force_type)
{
	bool ret;
	if (force_type) type = Type::MAT4;
	if (ret = (type == Type::MAT4)) value.mat4_v = mat4_v;
	return ret;
}

bool RE_Shader_Cvar::SetSampler(const char* res_ptr, bool force_type)
{
	bool ret;
	if (force_type) type = Type::SAMPLER;
	if (ret = (type == Type::SAMPLER)) value.char_p_v = res_ptr;
	return ret;
}

bool RE_Shader_Cvar::DrawPropieties(bool isInMemory)
{
	bool ret = false;
	eastl::string n = name;
	uint count = 0;
	float* fPtr = nullptr;
	switch (type)
	{
	case Type::BOOL:
		if(ImGui::Checkbox(name.c_str(), &value.bool_v))
			ret = true;
		break;
	case Type::BOOL2:
		n += eastl::to_string(count++);
		if (ImGui::Checkbox(n.c_str(), &value.bool2_v[0]))
			ret = true;
		n = name;		
		n += eastl::to_string(count++);
		if (ImGui::Checkbox(n.c_str(), &value.bool2_v[1]))
			ret = true;
		break;
	case Type::BOOL3:
		n += eastl::to_string(count++);
		if (ImGui::Checkbox(n.c_str(), &value.bool3_v[0]))
			ret = true;
		n = name;
		n += eastl::to_string(count++);
		if (ImGui::Checkbox(n.c_str(), &value.bool3_v[1]))
			ret = true;
		n = name;
		n += eastl::to_string(count++);
		if (ImGui::Checkbox(n.c_str(), &value.bool3_v[2]))
			ret = true;
		break;
	case Type::BOOL4:
		n += eastl::to_string(count++);
		if (ImGui::Checkbox(n.c_str(), &value.bool4_v[0]))
			ret = true;
		n = name;
		n += eastl::to_string(count++);
		if (ImGui::Checkbox(n.c_str(), &value.bool4_v[1]))
			ret = true;
		n = name;
		n += eastl::to_string(count++);
		if (ImGui::Checkbox(n.c_str(), &value.bool4_v[2]))
			ret = true;
		n = name;
		n += eastl::to_string(count++);
		if (ImGui::Checkbox(n.c_str(), &value.bool4_v[3]))
			ret = true;
		break;
	case Type::INT:
		if (ImGui::DragInt(name.c_str(), &value.int_v))
			ret = true;
		break;
	case Type::INT2:
		if (ImGui::DragInt2(name.c_str(), value.int2_v))
			ret = true;
		break;
	case Type::INT3:
		if (ImGui::DragInt3(name.c_str(), value.int3_v))
			ret = true;
		break;
	case Type::INT4:
		if (ImGui::DragInt4(name.c_str(), value.int4_v))
			ret = true;
		break;
	case Type::FLOAT:
		if (ImGui::DragFloat(name.c_str(), &value.float_v, 0.1f))
			ret = true;
		break;
	case Type::FLOAT2:
		if (ImGui::DragFloat2(name.c_str(), value.float2_v.ptr(), 0.1f))
			ret = true;
		break;
	case Type::FLOAT3:
		n += " as vector";
		if (ImGui::DragFloat3(n.c_str(), value.float3_v.ptr(), 0.1f))
			ret = true;
		n = name;
		n += " as color";
		if (ImGui::ColorEdit3(n.c_str(), value.float3_v.ptr()))
			ret = true;
		break;
	case Type::FLOAT4:
		n += " as vector";
		if (ImGui::DragFloat4(n.c_str(), value.float4_v.ptr(), 0.1f))
			ret = true;
		n = name;
		n += " as color";
		if (ImGui::ColorEdit4(n.c_str(), value.float4_v.ptr(), ImGuiColorEditFlags_::ImGuiColorEditFlags_AlphaPreview))
			ret = true;
		break;
	case Type::MAT2:
		fPtr = value.float4_v.ptr();
		n += eastl::to_string(count++);
		if (ImGui::DragFloat2(n.c_str(), fPtr, 0.1f))
			ret = true;
		n = name;
		n += eastl::to_string(count++);
		fPtr +=  2;
		if (ImGui::DragFloat2(n.c_str(), fPtr, 0.1f))
			ret = true;
		break;
	case Type::MAT3:
		fPtr = value.mat3_v.ptr();
		n += eastl::to_string(count++);
		if (ImGui::DragFloat3(n.c_str(), fPtr, 0.1f))
			ret = true;
		fPtr += 3;
		n = name;
		n += eastl::to_string(count++);
		if (ImGui::DragFloat3(n.c_str(), fPtr, 0.1f))
			ret = true;
		fPtr += 3;
		n = name;
		n += eastl::to_string(count++);
		if (ImGui::DragFloat3(n.c_str(), fPtr, 0.1f))
			ret = true;
		break;
	case Type::MAT4:
		fPtr = value.mat4_v.ptr();
		n += eastl::to_string(count++);
		if (ImGui::DragFloat4(n.c_str(), fPtr, 0.1f))
			ret = true;
		fPtr += 4;
		n = name;
		n += eastl::to_string(count++);
		if (ImGui::DragFloat4(n.c_str(), fPtr, 0.1f))
			ret = true;
		fPtr += 4;
		n = name;
		n += eastl::to_string(count++);
		if (ImGui::DragFloat4(n.c_str(), fPtr, 0.1f))
			ret = true;
		break;
	case Type::SAMPLER:
		ImGui::Text("Sampler: %s", name.c_str());
		if (!value.char_p_v)  ImGui::Text("No texture selected:");
		else
		{
			ResourceContainer* res = RE_RES->At((value.char_p_v));
			if (ImGui::Button(res->GetName()))
				RE_RES->PushSelected(res->GetMD5());

			ImGui::SameLine();
			if (ImGui::Button(eastl::string("Delete Sampler Texture #" + name).c_str()))
			{
				if (isInMemory) RE_RES->UnUse(value.char_p_v);
				value.char_p_v = nullptr;
				ret = true;
			}
		}

		if (ImGui::BeginMenu(eastl::string("Change Sampler Texture #" + name).c_str()))
		{
			eastl::vector<ResourceContainer*> allTex = RE_RES->GetResourcesByType(ResourceContainer::Type::TEXTURE);
			for (auto textRes : allTex)
			{
				if (ImGui::MenuItem(textRes->GetName()))
				{
					if (isInMemory) RE_RES->UnUse(value.char_p_v);
					value.char_p_v = textRes->GetMD5();
					if (isInMemory) RE_RES->Use(value.char_p_v);
					ret = true;
				}
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload("#TextureReference"))
			{
				if (isInMemory) RE_RES->UnUse(value.char_p_v);
				value.char_p_v = *static_cast<const char**>(dropped->Data);
				if (isInMemory) RE_RES->Use(value.char_p_v);
				ret = true;
			}
			ImGui::EndDragDropTarget();
		}
		break;
	}
	return ret;
}

#pragma endregion