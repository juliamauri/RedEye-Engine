#include "RE_Cvar.h"

#include "Application.h"
#include "RE_ResourceManager.h"
#include "RE_GameObject.h"

#include <ImGui/imgui.h>
#include <EASTL/vector.h>

RE_Cvar::RE_Cvar() : type(UNDEFINED) { value.int_v = 0; }

RE_Cvar::RE_Cvar(const RE_Cvar & copy) : type(copy.type)
{
	switch (copy.type)
	{
	case BOOL: value.bool_v = copy.value.bool_v; break;
	case BOOL2: memcpy(value.bool2_v,copy.value.bool2_v, sizeof(bool) *2); break;
	case BOOL3: memcpy(value.bool3_v, copy.value.bool3_v, sizeof(bool) * 3); break;
	case BOOL4: memcpy(value.bool4_v, copy.value.bool4_v, sizeof(bool) * 4); break;
	case SAMPLER:
	case INT: value.int_v = copy.value.int_v; break;
	case INT2: memcpy(value.int2_v, copy.value.int2_v, sizeof(int) * 2); break;
	case INT3: memcpy(value.int3_v, copy.value.int3_v, sizeof(int) * 3); break;
	case INT4: memcpy(value.int4_v, copy.value.int4_v, sizeof(int) * 4); break;
	case UINT: value.uint_v = copy.value.uint_v; break;
	case INT64: value.int64_v = copy.value.int64_v; break;
	case UINT64: value.uint64_v = copy.value.uint64_v; break;
	case DOUBLE: value.double_v = copy.value.double_v; break;
	case FLOAT: value.float_v = copy.value.float_v; break;
	case FLOAT2: value.float2_v = copy.value.float2_v; break;
	case FLOAT3: value.float3_v = copy.value.float3_v; break;
	case MAT2:
	case FLOAT4: value.float4_v = copy.value.float4_v; break;
	case CHAR_P: value.char_p_v = copy.value.char_p_v; break;
	case STRING: value.string_v = copy.value.string_v; break;
	case GAMEOBJECT: value.go_v = copy.value.go_v; break;
	case MAT3: value.mat3_v = copy.value.mat3_v; break;
	case MAT4: value.mat4_v = copy.value.mat4_v; break;
	}
}

RE_Cvar::RE_Cvar(bool bool_v) : type(BOOL) { value.bool_v = bool_v; }
RE_Cvar::RE_Cvar(int int_v) : type(INT) { value.int_v = int_v; }
RE_Cvar::RE_Cvar(unsigned int uint_v) : type(UINT) { value.uint_v = uint_v; }
RE_Cvar::RE_Cvar(long long int int64_v) : type(INT64) { value.int64_v = int64_v; }
RE_Cvar::RE_Cvar(unsigned long long int uint64_v) : type(UINT64) { value.uint64_v = uint64_v; }
RE_Cvar::RE_Cvar(double double_v) : type(DOUBLE) { value.double_v = double_v; }
RE_Cvar::RE_Cvar(float float_v) : type(FLOAT) { value.float_v = float_v; }
RE_Cvar::RE_Cvar(const char * char_p_v) : type(CHAR_P) { value.char_p_v = char_p_v; }
RE_Cvar::RE_Cvar(eastl::string string_v) : type(STRING) { value.string_v = string_v; }
RE_Cvar::RE_Cvar(RE_GameObject * go_v) : type(GAMEOBJECT) { value.go_v = go_v; }

RE_Cvar::RE_Cvar(const math::float4x4 mat4_v) : type(MAT4) { value.mat4_v = mat4_v; }

bool RE_Cvar::SetValue(bool bool_v, bool force_type)
{
	bool ret = false;
	if (force_type) type = BOOL;
	if (ret = (type == BOOL)) value.bool_v = bool_v;
	return ret;
}

bool RE_Cvar::SetValue(int int_v, bool force_type)
{
	bool ret = false;
	if (force_type) type = INT;
	if (ret = (type == INT)) value.int_v = int_v;
	return ret;
}

bool RE_Cvar::SetValue(unsigned int uint_v, bool force_type)
{
	bool ret = false;
	if (force_type) type = UINT;
	if (ret = (type == UINT)) value.uint_v = uint_v;
	return ret;
}

bool RE_Cvar::SetValue(long long int int64_v, bool force_type)
{
	bool ret = false;
	if (force_type) type = INT64;
	if (ret = (type == INT64)) value.int64_v = int64_v;
	return ret;
}

bool RE_Cvar::SetValue(unsigned long long int uint64_v, bool force_type)
{
	bool ret = false;
	if (force_type) type = UINT64;
	if (ret = (type == UINT64)) value.uint64_v = uint64_v;
	return ret;
}

bool RE_Cvar::SetValue(double double_v, bool force_type)
{
	bool ret = false;
	if (force_type) type = DOUBLE;
	if (ret = (type == DOUBLE)) value.double_v = double_v;
	return ret;
}

bool RE_Cvar::SetValue(float float_v, bool force_type)
{
	bool ret = false;
	if (force_type) type = FLOAT;
	if (ret = (type == FLOAT)) value.float_v = float_v;
	return ret;
}

bool RE_Cvar::SetValue(const char * char_p_v, bool force_type)
{
	bool ret = false;
	if (force_type) type = CHAR_P;
	if (ret = (type == CHAR_P)) value.char_p_v = char_p_v;
	return ret;
}

bool RE_Cvar::SetValue(eastl::string string_v, bool force_type)
{
	bool ret = false;
	if (force_type) type = STRING;
	if (ret = (type == STRING)) value.string_v = string_v;
	return ret;
}

bool RE_Cvar::SetValue(RE_GameObject * go_v, bool force_type)
{
	bool ret = false;
	if (force_type) type = GAMEOBJECT;
	if (ret = (type == GAMEOBJECT)) value.go_v = go_v;
	return ret;
}

RE_Cvar::VAR_TYPE RE_Cvar::GetType() const { return type; }
bool RE_Cvar::AsBool() const { return value.bool_v; }
bool* RE_Cvar::AsBool2() { return value.bool2_v; }
bool* RE_Cvar::AsBool3() { return value.bool3_v; }
bool* RE_Cvar::AsBool4() { return value.bool4_v; }
int RE_Cvar::AsInt() const { return value.int_v; }
int* RE_Cvar::AsInt2() {return value.int2_v; }
int* RE_Cvar::AsInt3() { return value.int3_v; }
int* RE_Cvar::AsInt4() { return value.int4_v; }
unsigned int RE_Cvar::AsUInt() const { return value.uint_v; }
long long int RE_Cvar::AsInt64() const { return value.int64_v; }
unsigned long long int RE_Cvar::AsUInt64() const { return value.uint64_v; }
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

	switch (type) {
	case RE_Cvar::FLOAT: ret = &value.float_v; break;
	case RE_Cvar::FLOAT2: ret = value.float2_v.ptr(); break;
	case RE_Cvar::FLOAT3: ret = value.float3_v.ptr(); break;
	case RE_Cvar::FLOAT4:
	case RE_Cvar::MAT2: ret = value.float4_v.ptr(); break;
	case RE_Cvar::MAT3: ret = value.mat3_v.ptr(); break;
	case RE_Cvar::MAT4: ret = value.mat4_v.ptr(); break; }

	return ret;
}

const float* RE_Cvar::AsFloatPointer() const
{
	const float* ret = nullptr;

	switch (type) {
	case RE_Cvar::FLOAT: ret = &value.float_v; break;
	case RE_Cvar::FLOAT2: ret = value.float2_v.ptr(); break;
	case RE_Cvar::FLOAT3: ret = value.float3_v.ptr(); break;
	case RE_Cvar::FLOAT4:
	case RE_Cvar::MAT2: ret = value.float4_v.ptr(); break;
	case RE_Cvar::MAT3: ret = value.mat3_v.ptr(); break;
	case RE_Cvar::MAT4: ret = value.mat4_v.ptr(); break;
	}

	return ret;
}

const char * RE_Cvar::AsCharP() const
{
	switch (type) {
	case RE_Cvar::CHAR_P: return value.char_p_v;
	case RE_Cvar::STRING: return value.string_v.c_str();
	default: return nullptr; }
}

RE_GameObject * RE_Cvar::AsGO() const { return value.go_v; }

// --- RE_Double_Cvar ------------------------------------------------------------------------------
/*RE_Double_Cvar::RE_Double_Cvar(bool bool_v) : RE_Cvar(bool_v) { original_value.bool_v = bool_v; }
RE_Double_Cvar::RE_Double_Cvar(int int_v) : RE_Cvar(int_v) { original_value.int_v = int_v; }
RE_Double_Cvar::RE_Double_Cvar(unsigned int uint_v) : RE_Cvar(uint_v) { original_value.uint_v = uint_v; }
RE_Double_Cvar::RE_Double_Cvar(long long int int64_v) : RE_Cvar(int64_v) { original_value.int64_v = int64_v; }
RE_Double_Cvar::RE_Double_Cvar(unsigned long long int uint64_v) : RE_Cvar(uint64_v) { original_value.uint64_v = uint64_v; }
RE_Double_Cvar::RE_Double_Cvar(double double_v) : RE_Cvar(double_v) { original_value.double_v = double_v; }
RE_Double_Cvar::RE_Double_Cvar(float float_v) : RE_Cvar(float_v) { original_value.float_v = float_v; }
RE_Double_Cvar::RE_Double_Cvar(const char * char_p_v) : RE_Cvar(char_p_v) { original_value.char_p_v = char_p_v; }

bool RE_Double_Cvar::ValueHasChanged() const
{
	bool ret = false;
	switch (type) {
	case BOOL: ret = (value.bool_v != original_value.bool_v); break;
	case INT: ret = (value.int_v != original_value.int_v); break;
	case UINT: ret = (value.uint_v != original_value.uint_v); break;
	case INT64: ret = (value.int64_v != original_value.int64_v); break;
	case UINT64: ret = (value.uint64_v != original_value.uint64_v); break;
	case DOUBLE: ret = (value.double_v != original_value.double_v); break;
	case FLOAT: ret = (value.float_v != original_value.float_v); break;
	case CHAR_P: ret = (value.char_p_v != original_value.char_p_v); break; }
	return ret;
}

bool RE_Double_Cvar::SetValue(bool bool_v, bool force_type)
{
	bool ret;
	if (force_type)
	{
		type = BOOL;
		original_value.bool_v = bool_v;
	}
	if (ret = (type == BOOL)) value.bool_v = bool_v;
	return ret;
}

bool RE_Double_Cvar::SetValue(int int_v, bool force_type)
{
	bool ret;
	if (force_type)
	{
		type = INT;
		original_value.int_v = int_v;
	}
	if (ret = (type == INT)) value.int_v = int_v;
	return ret;
}

bool RE_Double_Cvar::SetValue(unsigned int uint_v, bool force_type)
{
	bool ret;
	if (force_type)
	{
		type = UINT;
		original_value.uint_v = uint_v;
	}
	if (ret = (type == UINT)) value.uint_v = uint_v;
	return ret;
}

bool RE_Double_Cvar::SetValue(long long int int64_v, bool force_type)
{
	bool ret;
	if (force_type)
	{
		type = INT64;
		original_value.int64_v = int64_v;
	}
	if (ret = (type == INT64)) value.int64_v = int64_v;
	return ret;
}

bool RE_Double_Cvar::SetValue(unsigned long long int uint64_v, bool force_type)
{
	bool ret;
	if (force_type)
	{
		type = UINT64;
		original_value.uint64_v = uint64_v;
	}
	if (ret = (type == UINT64)) value.uint64_v = uint64_v;
	return ret;
}

bool RE_Double_Cvar::SetValue(double double_v, bool force_type)
{
	bool ret;
	if (force_type)
	{
		type = DOUBLE;
		original_value.double_v = double_v;
	}
	if (ret = (type == DOUBLE)) value.double_v = double_v;
	return ret;
}

bool RE_Double_Cvar::SetValue(float float_v, bool force_type)
{
	bool ret;
	if (force_type)
	{
		type = FLOAT;
		original_value.float_v = float_v;
	}
	if (ret = (type == FLOAT)) value.float_v = float_v;
	return ret;
}

bool RE_Double_Cvar::SetValue(const char * char_p_v, bool force_type)
{
	bool ret;
	if (force_type)
	{
		type = CHAR_P;
		original_value.char_p_v = char_p_v;
	}
	if (ret = (type == CHAR_P)) value.char_p_v = char_p_v;
	return ret;
}*/

RE_Shader_Cvar::RE_Shader_Cvar() : RE_Cvar() {}
RE_Shader_Cvar::RE_Shader_Cvar(const RE_Shader_Cvar& copy) : RE_Cvar(copy), name(copy.name), location(copy.location), locationDeferred(copy.locationDeferred), custom(copy.custom) { }
RE_Shader_Cvar::RE_Shader_Cvar(const bool bool_v) : RE_Cvar(bool_v) {}
RE_Shader_Cvar::RE_Shader_Cvar(const int int_v, bool sampler)
{
	RE_Cvar::type = (sampler) ? RE_Cvar::SAMPLER : RE_Cvar::INT;
	RE_Cvar::value.int_v = int_v;
}
RE_Shader_Cvar::RE_Shader_Cvar(float float_v) : RE_Cvar(float_v) {}
RE_Shader_Cvar::RE_Shader_Cvar(const bool boola_v[], unsigned int count)
{
	switch (count) {
	case 2: RE_Cvar::type = BOOL2; memcpy(value.bool2_v, boola_v, sizeof(bool) * 2); break;
	case 3: RE_Cvar::type = BOOL3; memcpy(value.bool3_v, boola_v, sizeof(bool) * 3); break;
	case 4: RE_Cvar::type = BOOL4; memcpy(value.bool4_v, boola_v, sizeof(bool) * 4); break;
	default: RE_Cvar::type = BOOL; value.bool_v = boola_v[0]; break; }
}

RE_Shader_Cvar::RE_Shader_Cvar(const int inta_v[], unsigned int count)
{
	switch (count) {
	case 2: RE_Cvar::type = INT2; memcpy(value.int2_v, inta_v, sizeof(int) * 2); break;
	case 3: RE_Cvar::type = INT3; memcpy(value.int3_v, inta_v, sizeof(int) * 3); break;
	case 4: RE_Cvar::type = INT4; memcpy(value.int4_v, inta_v, sizeof(int) * 4); break;
	default: RE_Cvar::type = INT; value.int_v = inta_v[0]; break; }
}

RE_Shader_Cvar::RE_Shader_Cvar(const math::float2 float2_v) {
	RE_Cvar::type = RE_Cvar::FLOAT2;
	RE_Cvar::value.float2_v = float2_v; }

RE_Shader_Cvar::RE_Shader_Cvar(const math::float3 float3_v) {
	RE_Cvar::type = RE_Cvar::FLOAT3;
	RE_Cvar::value.float3_v = float3_v; }

RE_Shader_Cvar::RE_Shader_Cvar(const math::float4 float4_v, bool mat2) {
	RE_Cvar::type = (mat2) ? RE_Cvar::MAT2 : RE_Cvar::FLOAT4;
	RE_Cvar::value.float4_v = float4_v; }

RE_Shader_Cvar::RE_Shader_Cvar(const math::float3x3 mat3_v) {
	RE_Cvar::type = RE_Cvar::MAT3;
	RE_Cvar::value.mat3_v = mat3_v; }

RE_Shader_Cvar::RE_Shader_Cvar(const math::float4x4 mat4_v) {
	RE_Cvar::type = RE_Cvar::MAT4;
	RE_Cvar::value.mat4_v = mat4_v; }

RE_Shader_Cvar RE_Shader_Cvar::operator=(const RE_Shader_Cvar& cpy)
{
	return RE_Shader_Cvar(cpy);
}

bool RE_Shader_Cvar::SetValue(const RE_Shader_Cvar& copyValue, bool force_type)
{
	bool ret = false;
	if (force_type) type = copyValue.type;

	if (ret = (type == copyValue.type)) {
		switch (type) {
		case RE_Cvar::BOOL: SetValue(copyValue.value.bool_v); break;
		case RE_Cvar::BOOL2: SetValue(&copyValue.value.bool2_v[0], 2); break;
		case RE_Cvar::BOOL3: SetValue(copyValue.value.bool3_v, 3); break;
		case RE_Cvar::BOOL4: SetValue(copyValue.value.bool4_v, 4); break;
		case RE_Cvar::INT: SetValue(copyValue.value.int_v); break;
		case RE_Cvar::INT2: SetValue(copyValue.value.int2_v, 2); break;
		case RE_Cvar::INT3: SetValue(copyValue.value.int3_v, 3); break;
		case RE_Cvar::INT4: SetValue(copyValue.value.int4_v, 4); break;
		case RE_Cvar::FLOAT: SetValue(copyValue.value.float_v); break;
		case RE_Cvar::FLOAT2: SetValue(copyValue.value.float2_v); break;
		case RE_Cvar::FLOAT3: SetValue(copyValue.value.float3_v); break;
		case RE_Cvar::FLOAT4: SetValue(copyValue.value.float4_v); break;
		case RE_Cvar::MAT2: SetValue(copyValue.value.float4_v); break;
		case RE_Cvar::MAT3: SetValue(copyValue.value.mat3_v); break;
		case RE_Cvar::MAT4: SetValue(copyValue.value.mat4_v); break;
		case RE_Cvar::SAMPLER: SetSampler(copyValue.value.char_p_v); break; } }

	return ret;
}

bool RE_Shader_Cvar::SetValue(const bool bool_v, bool force_type)
{
	bool ret;
	if (force_type) type = BOOL;
	if (ret = (type == BOOL)) value.bool_v =  bool_v;
	return ret;
}

bool RE_Shader_Cvar::SetValue(const bool boola_v[], unsigned int count, bool force_type)
{
	bool ret = false;

	RE_Cvar::VAR_TYPE toCheck;
	switch (count) {
	case 2: toCheck = BOOL2; break;
	case 3: toCheck = BOOL3; break;
	case 4: toCheck = BOOL4; break;
	default: toCheck = BOOL; break; }

	if (force_type) type = toCheck;

	if (ret = (type == toCheck)) {
		switch (count) {
		case 2: memcpy(value.bool2_v, boola_v, sizeof(bool) * 2); break;
		case 3: memcpy(value.bool3_v, boola_v, sizeof(bool) * 3); break;
		case 4: memcpy(value.bool4_v, boola_v, sizeof(bool) * 4); break;
		default: value.bool_v = boola_v[0]; break; } }

	return ret;
}

bool RE_Shader_Cvar::SetValue(const int int_v, bool force_type)
{
	bool ret;
	if (force_type) type = INT; 
	if (ret = (type == INT)) value.int_v = int_v;
	return ret;
}

bool RE_Shader_Cvar::SetValue(const int inta_v[], unsigned int count, bool force_type)
{
	bool ret;
	RE_Cvar::VAR_TYPE toCheck;
	switch (count) {
	case 2: toCheck = INT2; break;
	case 3: toCheck = INT3; break;
	case 4: toCheck = INT4; break;
	default: toCheck = INT; break; }

	if (force_type) type = toCheck;

	if (ret = (type == toCheck)) {
		switch (count) {
		case 2: memcpy(value.int2_v, inta_v, sizeof(int) * 2); break;
		case 3: memcpy(value.int3_v, inta_v, sizeof(int) * 3); break;
		case 4: memcpy(value.int4_v, inta_v, sizeof(int) * 4); break;
		default: value.int_v = inta_v[0]; break; } }

	return ret;
}

bool RE_Shader_Cvar::SetValue(const float float_v, bool force_type)
{
	bool ret;
	if (force_type) type = FLOAT; 
	if (ret = (type == FLOAT)) value.float_v = float_v;
	return ret;
}

bool RE_Shader_Cvar::SetValue(const math::float2 float2_v, bool force_type)
{
	bool ret;
	if (force_type) type = FLOAT2;
	if (ret = (type == FLOAT2)) value.float2_v = float2_v;
	return ret;
}

bool RE_Shader_Cvar::SetValue(const math::float3 float3_v, bool force_type)
{
	bool ret;
	if (force_type) type = FLOAT3;
	if (ret = (type == FLOAT3)) value.float3_v = float3_v;
	return ret;
}

bool RE_Shader_Cvar::SetValue(const math::float4 float4_v, bool mat2, bool force_type)
{
	bool ret;
	if (force_type) type = (mat2) ? MAT2 : FLOAT4;
	if (ret = (type == FLOAT4)) value.float4_v = float4_v;
	return ret;
}

bool RE_Shader_Cvar::SetValue(const math::float3x3 mat3_v, bool force_type)
{
	bool ret;
	if (force_type) type = MAT3;
	if (ret = (type == MAT3)) value.mat3_v = mat3_v;
	return ret;
}

bool RE_Shader_Cvar::SetValue(const math::float4x4 mat4_v, bool force_type)
{
	bool ret;
	if (force_type) type = MAT4;
	if (ret = (type == MAT4)) value.mat4_v = mat4_v;
	return ret;
}

bool RE_Shader_Cvar::SetSampler(const char* res_ptr, bool force_type)
{
	bool ret;
	if (force_type) type = SAMPLER;
	if (ret = (type == SAMPLER)) value.char_p_v = res_ptr;
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
	case RE_Cvar::BOOL:
		if(ImGui::Checkbox(name.c_str(), &value.bool_v))
			ret = true;
		break;
	case RE_Cvar::BOOL2:
		n += eastl::to_string(count++);
		if (ImGui::Checkbox(n.c_str(), &value.bool2_v[0]))
			ret = true;
		n = name;		
		n += eastl::to_string(count++);
		if (ImGui::Checkbox(n.c_str(), &value.bool2_v[1]))
			ret = true;
		break;
	case RE_Cvar::BOOL3:
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
	case RE_Cvar::BOOL4:
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
	case RE_Cvar::INT:
		if (ImGui::DragInt(name.c_str(), &value.int_v))
			ret = true;
		break;
	case RE_Cvar::INT2:
		if (ImGui::DragInt2(name.c_str(), value.int2_v))
			ret = true;
		break;
	case RE_Cvar::INT3:
		if (ImGui::DragInt3(name.c_str(), value.int3_v))
			ret = true;
		break;
	case RE_Cvar::INT4:
		if (ImGui::DragInt4(name.c_str(), value.int4_v))
			ret = true;
		break;
	case RE_Cvar::FLOAT:
		if (ImGui::DragFloat(name.c_str(), &value.float_v, 0.1f))
			ret = true;
		break;
	case RE_Cvar::FLOAT2:
		if (ImGui::DragFloat2(name.c_str(), value.float2_v.ptr(), 0.1f))
			ret = true;
		break;
	case RE_Cvar::FLOAT3:
		n += " as vector";
		if (ImGui::DragFloat3(n.c_str(), value.float3_v.ptr(), 0.1f))
			ret = true;
		n = name;
		n += " as color";
		if (ImGui::ColorEdit3(n.c_str(), value.float3_v.ptr()))
			ret = true;
		break;
	case RE_Cvar::FLOAT4:
		n += " as vector";
		if (ImGui::DragFloat4(n.c_str(), value.float4_v.ptr(), 0.1f))
			ret = true;
		n = name;
		n += " as color";
		if (ImGui::ColorEdit4(n.c_str(), value.float4_v.ptr(), ImGuiColorEditFlags_::ImGuiColorEditFlags_AlphaPreview))
			ret = true;
		break;
	case RE_Cvar::MAT2:
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
	case RE_Cvar::MAT3:
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
	case RE_Cvar::MAT4:
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
	case RE_Cvar::SAMPLER:
		ImGui::Text("Sampler: %s", name.c_str());
		if (!value.char_p_v)  ImGui::Text("No texture selected:");
		else {
			ResourceContainer* res = RE_RES->At((value.char_p_v));
			if (ImGui::Button(res->GetName())) RE_RES->PushSelected(res->GetMD5());

			ImGui::SameLine();
			if (ImGui::Button(eastl::string("Delete Sampler Texture #" + name).c_str())) {
				if (isInMemory) RE_RES->UnUse(value.char_p_v);
				value.char_p_v = nullptr;
				ret = true;
			}
		}

		if (ImGui::BeginMenu(eastl::string("Change Sampler Texture #" + name).c_str()))
		{
			eastl::vector<ResourceContainer*> allTex = RE_RES->GetResourcesByType(Resource_Type::R_TEXTURE);
			for (auto textRes : allTex) {
				if (ImGui::MenuItem(textRes->GetName())) {
					if (isInMemory) RE_RES->UnUse(value.char_p_v);
					value.char_p_v = textRes->GetMD5();
					if (isInMemory) RE_RES->Use(value.char_p_v);
					ret = true;
				}
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginDragDropTarget()) {

			if (const ImGuiPayload* dropped = ImGui::AcceptDragDropPayload("#TextureReference")) {
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
