#ifndef __RE_SERIALIZABLE_H__
#define __RE_SERIALIZABLE_H__

class RE_Json;

class RE_Serializable
{
public:

	RE_Serializable() = default;
	virtual ~RE_Serializable() = default;

	virtual void JsonSerialize(RE_Json* node) const = 0;
	virtual void JsonDeserialize(RE_Json* node) = 0;

	virtual size_t GetBinarySize() const = 0;
	virtual void BinarySerialize(char*& cursor) const = 0;
	virtual void BinaryDeserialize(char*& cursor) = 0;
};

#endif // !__RE_SERIALIZABLE_H__