#ifndef __RE_CURVE_H__
#define __RE_CURVE_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"

#include <ImGui/imgui.h>
#include <EASTL/vector.h>

struct RE_Curve : RE_Serializable
{
	RE_Curve();
	~RE_Curve() final;

	bool smooth = false;
	int total_points = 10;
	eastl::vector<ImVec2> points = {};
	int comboCurve = 0;

	float GetValue(const float weight) const;
	bool DrawEditor(const char* name);

	void JsonSerialize(RE_Json* node) const final;
	void JsonDeserialize(RE_Json* node) final;

	size_t GetBinarySize() const final;
	void BinarySerialize(char*& cursor) const final;
	void BinaryDeserialize(char*& cursor) final;
};

#endif // !__RE_CURVE_H__