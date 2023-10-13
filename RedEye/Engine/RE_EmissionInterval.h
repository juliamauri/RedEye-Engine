#ifndef __RE_EMISSION_INTERVAL_H__
#define __RE_EMISSION_INTERVAL_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"

class RE_EmissionInterval : public RE_Serializable
{
public:

	enum class Type : ushort
	{
		NONE = 0,
		INTERMITENT,
		CUSTOM
	};

public:

	Type type = Type(0);
	bool is_open = false;
	float time_offset = 0.f;
	float duration[2] = { 1.f, 1.f };

public:

	RE_EmissionInterval() = default;
	~RE_EmissionInterval() = default;

	bool DrawEditor(bool& changes);

	bool IsActive(float& dt);

	// Serialization
	void JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources = nullptr) const final;
	void JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources = nullptr) final;
	size_t GetBinarySize() const final;
	void BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources = nullptr) const final;
	void BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources = nullptr) final;
};

#endif // !__RE_EMISSION_INTERVAL_H__