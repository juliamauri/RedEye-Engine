#include "RE_Json.h"

#include "Globals.h"
#include "RE_Config.h"
#include "RapidJson\include\pointer.h"

RE_Json::RE_Json(const char* path, Config* config, bool isArray) : pointerPath(path), config(config)
{
	if (isArray) rapidjson::Pointer(path).Get(config->document)->SetArray();
}

RE_Json::RE_Json(RE_Json& node) : pointerPath(node.pointerPath), config(node.config) {}
RE_Json::~RE_Json() { config = nullptr; }


// Push ============================================================

void RE_Json::PushBool(const char* name, const bool value)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void RE_Json::PushBool(const char* name, const bool* value, unsigned int quantity)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value float_array(rapidjson::kArrayType);
		for (uint i = 0; i < quantity; i++) float_array.PushBack(value[i], config->document.GetAllocator());
		rapidjson::Pointer(path.c_str()).Set(config->document, float_array);
	}
}

void RE_Json::PushInt(const char* name, const int value)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void RE_Json::PushInt(const char* name, const int* value, unsigned int quantity)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value float_array(rapidjson::kArrayType);
		for (uint i = 0; i < quantity; i++) float_array.PushBack(value[i], config->document.GetAllocator());
		rapidjson::Pointer(path.c_str()).Set(config->document, float_array);
	}
}

void RE_Json::PushUInt(const char* name, const unsigned int value)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void RE_Json::PushUInt(const char* name, const unsigned int* value, unsigned int quantity)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value float_array(rapidjson::kArrayType);
		for (uint i = 0; i < quantity; i++) float_array.PushBack(value[i], config->document.GetAllocator());
		rapidjson::Pointer(path.c_str()).Set(config->document, float_array);
	}
}

void RE_Json::PushFloat(const char* name, const float value)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void RE_Json::PushFloat(const char* name, math::float2 value)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value float_array(rapidjson::kArrayType);
		float_array.PushBack(value.x, config->document.GetAllocator()).PushBack(value.y, config->document.GetAllocator());
		rapidjson::Pointer(path.c_str()).Set(config->document, float_array);
	}
}

void RE_Json::PushFloatVector(const char* name, math::vec vector)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value float_array(rapidjson::kArrayType);
		float_array.PushBack(vector.x, config->document.GetAllocator()).PushBack(vector.y, config->document.GetAllocator()).PushBack(vector.z, config->document.GetAllocator());
		rapidjson::Pointer(path.c_str()).Set(config->document, float_array);
	}
}

void RE_Json::PushFloat4(const char* name, math::float4 vector)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value float_array(rapidjson::kArrayType);
		float_array.PushBack(vector.x, config->document.GetAllocator()).PushBack(vector.y, config->document.GetAllocator()).PushBack(vector.z, config->document.GetAllocator()).PushBack(vector.w, config->document.GetAllocator());
		rapidjson::Pointer(path.c_str()).Set(config->document, float_array);
	}
}

void RE_Json::PushMat3(const char* name, math::float3x3 mat3)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value float_array(rapidjson::kArrayType);
		for (uint i = 0; i < 9; i++) float_array.PushBack(mat3.ptr()[i], config->document.GetAllocator());
		rapidjson::Pointer(path.c_str()).Set(config->document, float_array);
	}
}

void RE_Json::PushMat4(const char* name, math::float4x4 mat4)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value float_array(rapidjson::kArrayType);
		for (uint i = 0; i < 16; i++) float_array.PushBack(mat4.ptr()[i], config->document.GetAllocator());
		rapidjson::Pointer(path.c_str()).Set(config->document, float_array);
	}
}

void RE_Json::PushDouble(const char* name, const double value)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void RE_Json::PushSignedLongLong(const char* name, const signed long long value)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void RE_Json::PushUnsignedLongLong(const char* name, const unsigned long long value)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void RE_Json::PushString(const char* name, const char* value)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void RE_Json::PushValue(rapidjson::Value* val)
{
	rapidjson::Value* val_push = rapidjson::Pointer(pointerPath.c_str()).Get(config->document);
	if (val_push->IsArray()) val_push->PushBack(*val, config->document.GetAllocator());
}

RE_Json* RE_Json::PushJObject(const char* name)
{
	RE_Json* ret = nullptr;

	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		ret = new RE_Json(path.c_str(), config);
	}

	return ret;
}

// Pull ============================================================

bool RE_Json::PullBool(const char* name, bool deflt)
{
	bool ret = deflt;

	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);
		if (val) ret = val->GetBool();
	}

	return ret;
}

bool* RE_Json::PullBool(const char* name, unsigned int quantity, bool deflt)
{
	bool* ret = new bool[quantity];
	bool* cursor = ret;

	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);
		if (val)
		{
			for (uint i = 0; i < quantity; i++, cursor++)
			{
				bool f = val->GetArray()[i].GetBool();
				memcpy(cursor, &f, sizeof(bool));
			}
		}
		else
		{
			bool f = deflt;
			for (uint i = 0; i < quantity; i++, cursor++) memcpy(cursor, &f, sizeof(bool));
		}
	}

	return ret;
}

int RE_Json::PullInt(const char* name, int deflt)
{
	int ret = deflt;

	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);
		if (val) ret = val->GetInt();
	}

	return ret;
}

int* RE_Json::PullInt(const char* name, unsigned int quantity, int deflt)
{
	int* ret = new int[quantity];
	int* cursor = ret;

	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);

		if (val)
		{
			for (uint i = 0; i < quantity; i++, cursor++)
			{
				int f = val->GetArray()[i].GetInt();
				memcpy(cursor, &f, sizeof(int));
			}
		}
		else
		{
			int f = deflt;
			for (uint i = 0; i < quantity; i++, cursor++) memcpy(cursor, &f, sizeof(int));
		}
	}

	return ret;
}

unsigned int RE_Json::PullUInt(const char* name, const unsigned int deflt)
{
	unsigned int ret = 0u;

	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);
		if (val) ret = val->GetUint();
	}

	return ret;
}

unsigned int* RE_Json::PullUInt(const char* name, unsigned int quantity, unsigned int deflt)
{
	unsigned int* ret = new unsigned int[quantity];
	unsigned int* cursor = ret;

	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);

		if (val)
		{
			for (uint i = 0; i < quantity; i++, cursor++)
			{
				int f = val->GetArray()[i].GetUint();
				memcpy(cursor, &f, sizeof(unsigned int));
			}
		}
		else
		{
			int f = deflt;
			for (uint i = 0; i < quantity; i++, cursor++) memcpy(cursor, &f, sizeof(unsigned int));
		}
	}

	return ret;
}


float RE_Json::PullFloat(const char* name, float deflt)
{
	float ret = deflt;

	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);
		if (val) ret = val->GetFloat();
	}

	return ret;
}

math::float2 RE_Json::PullFloat(const char* name, math::float2 deflt)
{
	math::float2 ret = deflt;

	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);
		if (val) ret.Set(val->GetArray()[0].GetFloat(), val->GetArray()[1].GetFloat());
	}

	return ret;
}

math::vec RE_Json::PullFloatVector(const char* name, math::vec deflt)
{
	math::vec ret = deflt;

	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);
		if (val) ret.Set(val->GetArray()[0].GetFloat(), val->GetArray()[1].GetFloat(), val->GetArray()[2].GetFloat());
	}

	return ret;
}

math::float4 RE_Json::PullFloat4(const char* name, math::float4 deflt)
{
	math::float4 ret = deflt;

	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);
		if (val) ret.Set(val->GetArray()[0].GetFloat(), val->GetArray()[1].GetFloat(), val->GetArray()[2].GetFloat(), val->GetArray()[3].GetFloat());
	}

	return ret;
}

math::float3x3 RE_Json::PullMat3(const char* name, math::float3x3 deflt)
{
	math::float3x3 ret = deflt;

	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);

		if (val)
		{
			float* fBuffer = new float[9];
			float* cursor = fBuffer;

			for (uint i = 0; i < 9; i++, cursor++)
			{
				int f = val->GetArray()[i].GetInt();
				memcpy(cursor, &f, sizeof(int));
			}

			ret.Set(fBuffer);
			DEL_A(fBuffer);
		}
	}

	return ret;
}

math::float4x4 RE_Json::PullMat4(const char* name, math::float4x4 deflt)
{
	math::float4x4 ret = deflt;

	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);

		if (val != nullptr)
		{
			float* fBuffer = new float[16];
			float* cursor = fBuffer;

			for (uint i = 0; i < 16; i++, cursor++)
			{
				int f = val->GetArray()[i].GetInt();
				memcpy(cursor, &f, sizeof(int));
			}

			ret.Set(fBuffer);
			DEL_A(fBuffer);
		}
	}

	return ret;
}

double RE_Json::PullDouble(const char* name, double deflt)
{
	double ret = deflt;

	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);
		if (val) ret = val->GetDouble();
	}

	return ret;
}

signed long long RE_Json::PullSignedLongLong(const char* name, signed long long deflt)
{
	signed long long ret = deflt;

	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);
		if (val) ret = val->GetInt64();
	}

	return ret;
}

unsigned long long RE_Json::PullUnsignedLongLong(const char* name, unsigned long long deflt)
{
	unsigned long long ret = deflt;

	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);
		if (val) ret = val->GetUint64();
	}

	return ret;
}

const char* RE_Json::PullString(const char* name, const char* deflt)
{
	const char* ret = deflt;

	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);
		if (val) ret = val->GetString();
	}

	return ret;
}

RE_Json* RE_Json::PullJObject(const char* name)
{
	RE_Json* ret = nullptr;

	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		ret = new RE_Json(path.c_str(), config);
	}

	return ret;
}

rapidjson::Value::Array RE_Json::PullValueArray() { return config->document.FindMember(pointerPath.c_str())->value.GetArray(); }
inline bool RE_Json::operator!() const { return config || pointerPath.empty(); }
const char* RE_Json::GetDocumentPath() const { return pointerPath.c_str(); }
rapidjson::Document* RE_Json::GetDocument() { return &config->document; }

void RE_Json::SetArray() { rapidjson::Pointer(pointerPath.c_str()).Get(config->document)->SetArray(); }
void RE_Json::SetObject() { rapidjson::Pointer(pointerPath.c_str()).Get(config->document)->SetObject(); }
