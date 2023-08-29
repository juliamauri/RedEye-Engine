#ifndef __RE_JSON__
#define __RE_JSON__

#include "RE_DataTypes.h"
#include "RE_Config.h"
#include <RapidJson/rapidjson.h>
#include <RapidJson/document.h>
#include <RapidJson/pointer.h>
#include <MGL/Math/float2.h>
#include <MGL/Math/float3.h>
#include <MGL/Math/float4.h>
#include <MGL/Math/float3x3.h>
#include <MGL/Math/float4x4.h>
#include <EASTL/string.h>

class Config;

class RE_Json
{
public:

	RE_Json(const char* path = nullptr, Config* config = nullptr, bool isArray = false);
	~RE_Json();

	// Push
	template <typename T> void Push(const char* name, const T value);
	template <typename T> void Push(const char* name, const T* value, uint quantity);

	void		PushSizeT(const char* name, const size_t value);
	void		PushSizeT(const char* name, const size_t* value, uint quantity);

	void		PushFloat2(const char* name, math::float2 value);
	void		PushFloatVector(const char* name, math::vec vector);
	void		PushFloat4(const char* name, math::float4 vector);
	void		PushMat3(const char* name, math::float3x3 mat3);
	void		PushMat4(const char* name, math::float4x4 mat4);

	void		PushValue(rapidjson::Value* val);
	RE_Json*	PushJObject(const char* name);

	// Pull
	bool					PullBool(const char* name, bool deflt);
	bool *					PullBool(const char* name, uint quantity, bool deflt);
	int						PullInt(const char* name, int deflt);
	int *					PullInt(const char* name, uint quantity, int deflt);
	unsigned int			PullUInt(const char* name, uint deflt);
	unsigned int *			PullUInt(const char* name, uint quantity, uint deflt);

	size_t					PullSizeT(const char* name, size_t deflt);
	size_t *				PullSizeT(const char* name, uint quantity, size_t deflt);

	float					PullFloat(const char* name, float deflt);
	math::float2			PullFloat2(const char* name, math::float2 deflt);
	math::vec				PullFloatVector(const char* name, math::vec deflt);
	math::float4			PullFloat4(const char* name, math::float4 deflt);
	math::float3x3			PullMat3(const char* name, math::float3x3 deflt);
	math::float4x4			PullMat4(const char* name, math::float4x4 deflt);
	double					PullDouble(const char* name, double deflt);
	signed long long		PullSignedLongLong(const char* name, signed long long deflt);
	unsigned long long		PullUnsignedLongLong(const char* name, unsigned long long deflt);
	const char *			PullString(const char* name, const char* deflt);
	RE_Json *				PullJObject(const char* name);
	rapidjson::Value::Array	PullValueArray();

	// Utility
	inline bool operator!() const;
	const char* GetDocumentPath() const;
	rapidjson::Document* GetDocument();

	void SetArray();
	void SetObject();

private:

	RE_Json(RE_Json& node);

private:

	Config* config = nullptr;
	eastl::string pointerPath;
};

template<typename T>
inline void RE_Json::Push(const char* name, const T value)
{
	if (!name) return;
	eastl::string path = pointerPath + "/" + name;
	rapidjson::Pointer(path.c_str()).Set(config->document, value);
}

template<typename T>
inline void RE_Json::Push(const char* name, const T* value, uint quantity)
{
	if (!name) return;
	eastl::string path = pointerPath + "/" + name;
	rapidjson::Value values_array(rapidjson::kArrayType);
	for (uint i = 0; i < quantity; i++) values_array.PushBack(value[i], config->document.GetAllocator());
	rapidjson::Pointer(path.c_str()).Set(config->document, values_array);
}

#endif // !__RE_JSON__