#include "JSONNode.h"

#include "Globals.h"
#include "RE_Config.h"
#include "RapidJson\include\pointer.h"

JSONNode::JSONNode(const char* path, Config* config, bool isArray) : pointerPath(path), config(config)
{
	if (isArray) rapidjson::Pointer(path).Get(config->document)->SetArray();
}

JSONNode::JSONNode(JSONNode& node) : pointerPath(node.pointerPath), config(node.config) {}
JSONNode::~JSONNode() { config = nullptr; }


// Push ============================================================

void JSONNode::PushBool(const char* name, const bool value)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void JSONNode::PushBool(const char* name, const bool* value, unsigned int quantity)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value float_array(rapidjson::kArrayType);
		for (uint i = 0; i < quantity; i++) float_array.PushBack(value[i], config->document.GetAllocator());
		rapidjson::Pointer(path.c_str()).Set(config->document, float_array);
	}
}

void JSONNode::PushInt(const char* name, const int value)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void JSONNode::PushInt(const char* name, const int* value, unsigned int quantity)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value float_array(rapidjson::kArrayType);
		for (uint i = 0; i < quantity; i++) float_array.PushBack(value[i], config->document.GetAllocator());
		rapidjson::Pointer(path.c_str()).Set(config->document, float_array);
	}
}

void JSONNode::PushUInt(const char* name, const unsigned int value)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void JSONNode::PushUInt(const char* name, const unsigned int* value, unsigned int quantity)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value float_array(rapidjson::kArrayType);
		for (uint i = 0; i < quantity; i++) float_array.PushBack(value[i], config->document.GetAllocator());
		rapidjson::Pointer(path.c_str()).Set(config->document, float_array);
	}
}

void JSONNode::PushFloat(const char* name, const float value)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void JSONNode::PushFloat(const char* name, math::float2 value)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value float_array(rapidjson::kArrayType);
		float_array.PushBack(value.x, config->document.GetAllocator()).PushBack(value.y, config->document.GetAllocator());
		rapidjson::Pointer(path.c_str()).Set(config->document, float_array);
	}
}

void JSONNode::PushFloatVector(const char* name, math::vec vector)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value float_array(rapidjson::kArrayType);
		float_array.PushBack(vector.x, config->document.GetAllocator()).PushBack(vector.y, config->document.GetAllocator()).PushBack(vector.z, config->document.GetAllocator());
		rapidjson::Pointer(path.c_str()).Set(config->document, float_array);
	}
}

void JSONNode::PushFloat4(const char* name, math::float4 vector)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value float_array(rapidjson::kArrayType);
		float_array.PushBack(vector.x, config->document.GetAllocator()).PushBack(vector.y, config->document.GetAllocator()).PushBack(vector.z, config->document.GetAllocator()).PushBack(vector.w, config->document.GetAllocator());
		rapidjson::Pointer(path.c_str()).Set(config->document, float_array);
	}
}

void JSONNode::PushMat3(const char* name, math::float3x3 mat3)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value float_array(rapidjson::kArrayType);
		for (uint i = 0; i < 9; i++) float_array.PushBack(mat3.ptr()[i], config->document.GetAllocator());
		rapidjson::Pointer(path.c_str()).Set(config->document, float_array);
	}
}

void JSONNode::PushMat4(const char* name, math::float4x4 mat4)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Value float_array(rapidjson::kArrayType);
		for (uint i = 0; i < 16; i++) float_array.PushBack(mat4.ptr()[i], config->document.GetAllocator());
		rapidjson::Pointer(path.c_str()).Set(config->document, float_array);
	}
}

void JSONNode::PushDouble(const char* name, const double value)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void JSONNode::PushSignedLongLong(const char* name, const signed long long value)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void JSONNode::PushUnsignedLongLong(const char* name, const unsigned long long value)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void JSONNode::PushString(const char* name, const char* value)
{
	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void JSONNode::PushValue(rapidjson::Value* val)
{
	rapidjson::Value* val_push = rapidjson::Pointer(pointerPath.c_str()).Get(config->document);
	if (val_push->IsArray()) val_push->PushBack(*val, config->document.GetAllocator());
}

JSONNode* JSONNode::PushJObject(const char* name)
{
	JSONNode* ret = nullptr;

	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		ret = new JSONNode(path.c_str(), config);
	}

	return ret;
}

// Pull ============================================================

bool JSONNode::PullBool(const char* name, bool deflt)
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

bool* JSONNode::PullBool(const char* name, unsigned int quantity, bool deflt)
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

int JSONNode::PullInt(const char* name, int deflt)
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

int* JSONNode::PullInt(const char* name, unsigned int quantity, int deflt)
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

unsigned int JSONNode::PullUInt(const char* name, const unsigned int deflt)
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

unsigned int* JSONNode::PullUInt(const char* name, unsigned int quantity, unsigned int deflt)
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


float JSONNode::PullFloat(const char* name, float deflt)
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

math::float2 JSONNode::PullFloat(const char* name, math::float2 deflt)
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

math::vec JSONNode::PullFloatVector(const char* name, math::vec deflt)
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

math::float4 JSONNode::PullFloat4(const char* name, math::float4 deflt)
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

math::float3x3 JSONNode::PullMat3(const char* name, math::float3x3 deflt)
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

math::float4x4 JSONNode::PullMat4(const char* name, math::float4x4 deflt)
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

double JSONNode::PullDouble(const char* name, double deflt)
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

signed long long JSONNode::PullSignedLongLong(const char* name, signed long long deflt)
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

unsigned long long JSONNode::PullUnsignedLongLong(const char* name, unsigned long long deflt)
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

const char* JSONNode::PullString(const char* name, const char* deflt)
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

JSONNode* JSONNode::PullJObject(const char* name)
{
	JSONNode* ret = nullptr;

	if (name)
	{
		eastl::string path = pointerPath + "/" + name;
		ret = new JSONNode(path.c_str(), config);
	}

	return ret;
}

rapidjson::Value::Array JSONNode::PullValueArray() { return config->document.FindMember(pointerPath.c_str())->value.GetArray(); }
inline bool JSONNode::operator!() const { return config || pointerPath.empty(); }
const char* JSONNode::GetDocumentPath() const { return pointerPath.c_str(); }
rapidjson::Document* JSONNode::GetDocument() { return &config->document; }

void JSONNode::SetArray() { rapidjson::Pointer(pointerPath.c_str()).Get(config->document)->SetArray(); }
void JSONNode::SetObject() { rapidjson::Pointer(pointerPath.c_str()).Get(config->document)->SetObject(); }
