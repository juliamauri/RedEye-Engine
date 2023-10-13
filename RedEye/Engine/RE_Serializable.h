#ifndef __RE_SERIALIZABLE_H__
#define __RE_SERIALIZABLE_H__

#include <EASTL/map.h>

class RE_Json;

class RE_Serializable
{
public:

	RE_Serializable() = default;
	virtual ~RE_Serializable() = default;

	virtual void JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources = nullptr) const = 0;
	virtual void JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources = nullptr) = 0;

	virtual size_t GetBinarySize() const = 0;
	virtual void BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources = nullptr) const = 0;
	virtual void BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources = nullptr) = 0;
};

#endif // !__RE_SERIALIZABLE_H__