#include "Cvar.h"

#include "RE_GameObject.h"

Cvar::Cvar() : type(UNDEFINED) { value.int_v = 0; }

Cvar::Cvar(Cvar & copy) : type(copy.type)
{
	switch (copy.type)
	{
	case BOOL: value.bool_v = copy.value.bool_v; break;
	case INT: value.int_v = copy.value.int_v; break;
	case UINT: value.uint_v = copy.value.uint_v; break;
	case INT64: value.int64_v = copy.value.int64_v; break;
	case UINT64: value.uint64_v = copy.value.uint64_v; break;
	case DOUBLE: value.double_v = copy.value.double_v; break;
	case FLOAT: value.float_v = copy.value.float_v; break;
	case CHAR_P: value.char_p_v = copy.value.char_p_v; break;
	case GAMEOBJECT: value.go_v = copy.value.go_v; break;
	}
}

Cvar::Cvar(bool bool_v) : type(BOOL) { value.bool_v = bool_v; }

Cvar::Cvar(int int_v) : type(INT) { value.int_v = int_v; }

Cvar::Cvar(unsigned int uint_v) : type(UINT) { value.uint_v = uint_v; }

Cvar::Cvar(long long int int64_v) : type(INT64) { value.int64_v = int64_v; }

Cvar::Cvar(unsigned long long int uint64_v) : type(UINT64) { value.uint64_v = uint64_v; }

Cvar::Cvar(double double_v) : type(DOUBLE) { value.double_v = double_v; }

Cvar::Cvar(float float_v) : type(FLOAT) { value.float_v = float_v; }

Cvar::Cvar(const char * char_p_v) : type(CHAR_P) { value.char_p_v = char_p_v; }

Cvar::Cvar(RE_GameObject * go_v) : type(GAMEOBJECT) { value.go_v = go_v; }

bool Cvar::SetValue(bool bool_v, bool force_type)
{
	bool ret = false;

	if (force_type)
		type = BOOL;

	if (ret = (type == BOOL))
		value.bool_v = bool_v;

	return ret;
}

bool Cvar::SetValue(int int_v, bool force_type)
{
	bool ret = false;

	if (force_type)
		type = INT;

	if (ret = (type == INT))
		value.int_v = int_v;
	 
	return ret;
}

bool Cvar::SetValue(unsigned int uint_v, bool force_type)
{
	bool ret = false;

	if (force_type)
		type = UINT;

	if (ret = (type == UINT))
		value.uint_v = uint_v;

	return ret;
}

bool Cvar::SetValue(long long int int64_v, bool force_type)
{
	bool ret = false;

	if (force_type)
		type = INT64;

	if (ret = (type == INT64))
		value.int64_v = int64_v;

	return ret;
}

bool Cvar::SetValue(unsigned long long int uint64_v, bool force_type)
{
	bool ret = false;

	if (force_type)
		type = UINT64;

	if (ret = (type == UINT64))
		value.uint64_v = uint64_v;

	return ret;
}

bool Cvar::SetValue(double double_v, bool force_type)
{
	bool ret = false;

	if (force_type)
		type = DOUBLE;

	if (ret = (type == DOUBLE))
		value.double_v = double_v;

	return ret;
}

bool Cvar::SetValue(float float_v, bool force_type)
{
	bool ret = false;

	if (force_type)
		type = FLOAT;

	if (ret = (type == FLOAT))
		value.float_v = float_v;

	return ret;
}

bool Cvar::SetValue(const char * char_p_v, bool force_type)
{
	bool ret = false;

	if (force_type)
		type = CHAR_P;

	if (ret = (type == CHAR_P))
		value.char_p_v = char_p_v;

	return ret;
}

bool Cvar::SetValue(RE_GameObject * go_v, bool force_type)
{
	bool ret = false;

	if (force_type)
		type = GAMEOBJECT;

	if (ret = (type == GAMEOBJECT))
		value.go_v = go_v;

	return ret;
}

Cvar::VAR_TYPE Cvar::GetType() const { return type; }

bool Cvar::AsBool() const { return value.bool_v; }

int Cvar::AsInt() const { return value.int_v; }

unsigned int Cvar::AsUInt() const { return value.uint_v; }

long long int Cvar::AsInt64() const { return value.int64_v; }

unsigned long long int Cvar::AsUInt64() const { return value.uint64_v; }

double Cvar::AsDouble() const { return value.double_v; }

float Cvar::AsFloat() const { return value.float_v; }

const char * Cvar::AsCharP() const { return value.char_p_v; }

RE_GameObject * Cvar::AsGO() const
{
	return value.go_v;
}

// --- DoubleCvar ------------------------------------------------------------------------------
/*
DoubleCvar::DoubleCvar(bool bool_v) : Cvar(bool_v) { original_value.bool_v = bool_v; }

DoubleCvar::DoubleCvar(int int_v) : Cvar(int_v) { original_value.int_v = int_v; }

DoubleCvar::DoubleCvar(unsigned int uint_v) : Cvar(uint_v) { original_value.uint_v = uint_v; }

DoubleCvar::DoubleCvar(long long int int64_v) : Cvar(int64_v) { original_value.int64_v = int64_v; }

DoubleCvar::DoubleCvar(unsigned long long int uint64_v) : Cvar(uint64_v) { original_value.uint64_v = uint64_v; }

DoubleCvar::DoubleCvar(double double_v) : Cvar(double_v) { original_value.double_v = double_v; }

DoubleCvar::DoubleCvar(float float_v) : Cvar(float_v) { original_value.float_v = float_v; }

DoubleCvar::DoubleCvar(const char * char_p_v) : Cvar(char_p_v) { original_value.char_p_v = char_p_v; }

bool DoubleCvar::ValueHasChanged() const
{
	bool ret = false;

	switch (type)
	{
	case BOOL: ret = (value.bool_v != original_value.bool_v); break;
	case INT: ret = (value.int_v != original_value.int_v); break;
	case UINT: ret = (value.uint_v != original_value.uint_v); break;
	case INT64: ret = (value.int64_v != original_value.int64_v); break;
	case UINT64: ret = (value.uint64_v != original_value.uint64_v); break;
	case DOUBLE: ret = (value.double_v != original_value.double_v); break;
	case FLOAT: ret = (value.float_v != original_value.float_v); break;
	case CHAR_P: ret = (value.char_p_v != original_value.char_p_v); break;
	}

	return ret;
}

bool DoubleCvar::SetValue(bool bool_v, bool force_type)
{
	bool ret = false;

	if (force_type)
	{
		type = BOOL;
		original_value.bool_v = bool_v;
	}

	if (ret = (type == BOOL))
		value.bool_v = bool_v;

	return ret;
}

bool DoubleCvar::SetValue(int int_v, bool force_type)
{
	bool ret = false;

	if (force_type)
	{
		type = INT;
		original_value.int_v = int_v;
	}

	if (ret = (type == INT))
		value.int_v = int_v;

	return ret;
}

bool DoubleCvar::SetValue(unsigned int uint_v, bool force_type)
{
	bool ret = false;

	if (force_type)
	{
		type = UINT;
		original_value.uint_v = uint_v;
	}

	if (ret = (type == UINT))
		value.uint_v = uint_v;

	return ret;
}

bool DoubleCvar::SetValue(long long int int64_v, bool force_type)
{
	bool ret = false;

	if (force_type)
	{
		type = INT64;
		original_value.int64_v = int64_v;
	}

	if (ret = (type == INT64))
		value.int64_v = int64_v;

	return ret;
}

bool DoubleCvar::SetValue(unsigned long long int uint64_v, bool force_type)
{
	bool ret = false;

	if (force_type)
	{
		type = UINT64;
		original_value.uint64_v = uint64_v;
	}

	if (ret = (type == UINT64))
		value.uint64_v = uint64_v;

	return ret;
}

bool DoubleCvar::SetValue(double double_v, bool force_type)
{
	bool ret = false;

	if (force_type)
	{
		type = DOUBLE;
		original_value.double_v = double_v;
	}

	if (ret = (type == DOUBLE))
		value.double_v = double_v;

	return ret;
}

bool DoubleCvar::SetValue(float float_v, bool force_type)
{
	bool ret = false;

	if (force_type)
	{
		type = FLOAT;
		original_value.float_v = float_v;
	}

	if (ret = (type == FLOAT))
		value.float_v = float_v;

	return ret;
}

bool DoubleCvar::SetValue(const char * char_p_v, bool force_type)
{
	bool ret = false;

	if (force_type)
	{
		type = CHAR_P;
		original_value.char_p_v = char_p_v;
	}

	if (ret = (type == CHAR_P))
		value.char_p_v = char_p_v;

	return ret;
}
*/