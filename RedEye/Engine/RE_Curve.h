#ifndef __RE_CURVE_H__
#define __RE_CURVE_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"

#include <ImGui/imgui.h>
#include <EASTL/vector.h>

struct RE_Curve : RE_Serializable
{
	RE_Curve();
	~RE_Curve();

	bool smooth = false;
	int total_points = 10;
	eastl::vector<ImVec2> points = {};
	int comboCurve = 0;

	float GetValue(const float weight) const;
	bool DrawEditor(const char* name);

	void JsonSerialize(RE_Json* node) const override;
	void JsonDeserialize(RE_Json* node) override;

	size_t GetBinarySize() const override;
	void BinarySerialize(char*& cursor) const override;
	void BinaryDeserialize(char*& cursor) override;
};

#endif // !__RE_CURVE_H__