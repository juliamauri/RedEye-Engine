#include "RE_PR_Color.h"

#include "RE_Memory.h"
#include "RE_Json.h"

math::vec RE_PR_Color::GetValue(const float weight) const
{
	math::vec ret = math::vec::zero;

	switch (type)
	{
	case Type::SINGLE: ret = base;
	default:
	{
		float w = (useCurve) ? curve.GetValue(weight) : weight;
		ret = (gradient * w) + (base * (1.f - w));
	}
	}

	return ret;
}

bool RE_PR_Color::DrawEditor()
{
	bool ret = false;
	int tmp = static_cast<int>(type);
	if (ImGui::Combo("Color Type", &tmp, "Single\0Over Lifetime\0Over Distance\0Over Speed\0")) {
		type = static_cast<Type>(tmp);
		ret = true;
	}

	if (type == Type::SINGLE)
	{
		if (ImGui::ColorEdit3("Particle Color", base.ptr())) ret = true;
	}
	else
	{
		if (ImGui::ColorEdit3("Particle Gradient 1", base.ptr()) ||
			ImGui::ColorEdit3("Particle Gradient 2", gradient.ptr()) ||
			ImGui::Checkbox(useCurve ? "Disable Color Curve" : "Enable Color Curve", &useCurve))
			ret = true;
	}

	return ret;
}

void RE_PR_Color::JsonSerialize(RE_Json* node) const
{
	node->Push("Type", static_cast<int>(type));

	node->PushFloatVector("Base", base);
	node->PushFloatVector("Gradient", gradient);
	node->Push("useCurve", useCurve);

	curve.JsonSerialize(node->PushJObject("curve"));

	DEL(node)
}

void RE_PR_Color::JsonDeserialize(RE_Json* node)
{
	type = static_cast<Type>(node->PullInt("Type", 0));

	base = node->PullFloatVector("Base", math::vec::one);
	gradient = node->PullFloatVector("Gradient", math::vec::zero);
	useCurve = node->PullBool("useCurve", false);

	curve.JsonDeserialize(node->PullJObject("curve"));

	DEL(node)
}

size_t RE_PR_Color::GetBinarySize() const
{
	return sizeof(ushort)
		+ (sizeof(float) * 6)
		+ sizeof(bool)
		+ curve.GetBinarySize();
}

void RE_PR_Color::BinarySerialize(char*& cursor) const
{
	size_t size = sizeof(Type);
	memcpy(cursor, &type, size);
	cursor += size;

	size = sizeof(float) * 3;
	memcpy(cursor, base.ptr(), size);
	cursor += size;

	memcpy(cursor, gradient.ptr(), size);
	cursor += size;

	size = sizeof(bool);
	memcpy(cursor, &useCurve, size);
	cursor += size;

	curve.BinarySerialize(cursor);
}

void RE_PR_Color::BinaryDeserialize(char*& cursor)
{
	size_t size = sizeof(Type);
	memcpy(&type, cursor, size);
	cursor += size;

	size = sizeof(float) * 3;
	memcpy(base.ptr(), cursor, size);
	cursor += size;

	memcpy(gradient.ptr(), cursor, size);
	cursor += size;

	size = sizeof(bool);
	memcpy(&useCurve, cursor, size);
	cursor += size;

	curve.BinaryDeserialize(cursor);
}