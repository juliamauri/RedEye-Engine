#ifndef __RE_JSON__
#define __RE_JSON__

#include "RapidJson\include\rapidjson.h"
#include "RapidJson\include\document.h"
#include "MathGeoLib\include\Math\float2.h"
#include "MathGeoLib\include\Math\float3.h"
#include "MathGeoLib\include\Math\float4.h"
#include "MathGeoLib\include\Math\float3x3.h"
#include "MathGeoLib\include\Math\float4x4.h"
#include <EASTL\string.h>

class Config;

class RE_Json
{
public:

	RE_Json(const char* path = nullptr, Config* config = nullptr, bool isArray = false);
	~RE_Json();

	// Push
	void		PushBool(const char* name, const bool value);
	void		PushBool(const char* name, const bool* value, unsigned int quantity);
	void		PushInt(const char* name, const int value);
	void		PushInt(const char* name, const int* value, unsigned int quantity);
	void		PushUInt(const char* name, const unsigned int value);
	void		PushUInt(const char* name, const unsigned int* value, unsigned int quantity);
	void		PushFloat(const char* name, const float value);
	void		PushFloat(const char* name, math::float2 value);
	void		PushFloatVector(const char* name, math::vec vector);
	void		PushFloat4(const char* name, math::float4 vector);
	void		PushMat3(const char* name, math::float3x3 mat3);
	void		PushMat4(const char* name, math::float4x4 mat4);
	void		PushDouble(const char* name, const double value);
	void		PushSignedLongLong(const char* name, const signed long long value);
	void		PushUnsignedLongLong(const char* name, const unsigned long long value);
	void		PushString(const char* name, const char* value);
	void		PushValue(rapidjson::Value* val);
	RE_Json*	PushJObject(const char* name);

	// Pull
	bool					PullBool(const char* name, bool deflt);
	bool *					PullBool(const char* name, unsigned int quantity, bool deflt);
	int						PullInt(const char* name, int deflt);
	int *					PullInt(const char* name, unsigned int quantity, int deflt);
	unsigned int			PullUInt(const char* name, unsigned int deflt);
	unsigned int *			PullUInt(const char* name, unsigned int quantity, unsigned int deflt);
	float					PullFloat(const char* name, float deflt);
	math::float2			PullFloat(const char* name, math::float2 deflt);
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

#endif // !__RE_JSON__