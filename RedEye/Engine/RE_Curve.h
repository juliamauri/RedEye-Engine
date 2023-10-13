#ifndef __RE_CURVE_H__
#define __RE_CURVE_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"

#include <ImGui/imgui.h>
#include <EASTL/vector.h>

class RE_Curve : public RE_Serializable
{
public:

	bool smooth = false;
	int total_points = 10;
	eastl::vector<ImVec2> points = {};
	int comboCurve = 0;

public:

	RE_Curve();
	~RE_Curve() final;

	float GetValue(const float weight) const;
	bool DrawEditor(const char* name);

	// Serialization
	void JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources = nullptr) const final;
	void JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources = nullptr) final;
	size_t GetBinarySize() const final;
	void BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources = nullptr) const final;
	void BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources = nullptr) final;
};

#endif // !__RE_CURVE_H__