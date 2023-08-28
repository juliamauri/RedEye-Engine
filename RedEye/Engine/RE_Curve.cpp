#include "RE_Curve.h"

#include "RE_Memory.h"
#include "RE_Json.h"

#include <ImGuiWidgets/ImGuiCurverEditor/ImGuiCurveEditor.hpp>

RE_Curve::RE_Curve()
{
	points.push_back({ -1.0f, 0.0f });
	for (int i = 1; i < total_points; i++)
		points.push_back({ 0.0f, 0.0f });
}

RE_Curve::~RE_Curve() { points.clear(); }

float RE_Curve::GetValue(const float weight) const
{
	return smooth ?
		ImGui::CurveValueSmooth(weight, total_points, points.data()) :
		ImGui::CurveValue(weight, total_points, points.data());
}

bool RE_Curve::DrawEditor(const char* name)
{
	bool ret = false;
	eastl::string tmp(name);
	static float cSize[2] = { 600.f, 200.f };

	ImGui::SameLine();
	ImGui::PushItemWidth(150.f);
	ImGui::DragFloat2((tmp + " curve size").c_str(), cSize, 1.0f, 0.0f, 0.0f, "%.0f");
	ImGui::PopItemWidth();
	if (ImGui::Curve((tmp + " curve").c_str(), { cSize[0], cSize[1] }, total_points, points.data(), &comboCurve)) ret = true;
	ImGui::SameLine();
	ImGui::PushItemWidth(50.f);

	static int minPoitns = 3;
	if (ImGui::DragInt((tmp + " num Points").c_str(), &total_points, 1.0f)) {

		if (total_points < minPoitns) total_points = minPoitns;

		points.clear();
		points.push_back({ -1.0f, 0.0f });
		for (int i = 1; i < total_points; i++)
			points.push_back({ 0.0f, 0.0f });

		ret = true;
	}

	ImGui::SameLine();
	if (ImGui::Checkbox((tmp + " smooth curve").c_str(), &smooth)) ret = true;

	return ret;
}

void RE_Curve::JsonSerialize(RE_Json* node) const
{
	node->Push("Smooth", smooth);
	node->Push("TotalPoints", total_points);
	node->Push("comboCurve", comboCurve);

	for (int i = 0; i < total_points; i++)
		node->PushFloat2((eastl::to_string(i) + "p").c_str(), { points[i].x, points[i].y });

	DEL(node);
}

void RE_Curve::JsonDeserialize(RE_Json* node)
{
	smooth = node->PullBool("Smooth", false);
	total_points = node->PullInt("TotalPoints", 10);
	comboCurve = node->PullInt("comboCurve", 0);

	for (int i = 0; i < total_points; i++)
	{
		math::float2 toImVec2 = node->PullFloat2((eastl::to_string(i) + "p").c_str(), { -1.0f, 0.0f });
		points.push_back({ toImVec2.x,toImVec2.y });
	}

	DEL(node);
}

size_t RE_Curve::GetBinarySize() const
{
	return sizeof(bool) + (sizeof(int) * 2) + (sizeof(float) * 2 * points.size());
}

void RE_Curve::BinarySerialize(char*& cursor) const
{
	size_t size = sizeof(bool);
	memcpy(cursor, &smooth, size);
	cursor += size;

	size = sizeof(int);
	memcpy(cursor, &total_points, size);
	cursor += size;

	memcpy(cursor, &comboCurve, size);
	cursor += size;

	size = sizeof(float) * 2u * total_points;
	memcpy(cursor, points.data(), size);
	cursor += size;
}

void RE_Curve::BinaryDeserialize(char*& cursor)
{
	size_t size = sizeof(bool);
	memcpy(&smooth, cursor, size);
	cursor += size;

	size = sizeof(int);
	memcpy(&total_points, cursor, size);
	cursor += size;

	memcpy(&comboCurve, cursor, size);
	cursor += size;

	points.clear();
	size = sizeof(float);
	float x = -1.0f, y = 0.0f;
	for (int i = 0; i < total_points; i++)
	{
		memcpy(&x, cursor, size);
		cursor += size;
		memcpy(&y, cursor, size);
		cursor += size;

		points.push_back({ x, y });
	}
}
