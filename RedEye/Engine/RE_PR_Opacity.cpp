#include "RE_PR_Opacity.h"

#include "RE_Memory.h"
#include "RE_Json.h"

float RE_PR_Opacity::GetValue(const float weight) const
{
	float ret = 1.f;

	switch (type)
	{
	case Type::VALUE: ret = opacity;
	case Type::OVERLIFETIME:
	case Type::OVERDISTANCE:
	case Type::OVERSPEED:
		ret = (useCurve) ? curve.GetValue(weight) : (inverted) ? 1.f - weight : weight;
	default: break;
	}

	return ret;
}

bool RE_PR_Opacity::DrawEditor()
{
	bool ret = false;
	int tmp = static_cast<int>(type);
	if (ImGui::Combo("Opacity Type", &tmp, "None\0Value\0Over Lifetime\0Over Distance\0Over Speed\0"))
	{
		type = static_cast<Type>(tmp);
		ret = true;
	}

	switch (type)
	{
	case Type::VALUE:
		if (ImGui::SliderFloat("Opacity", &opacity, 0.0f, 1.0f)) ret = true;
		break;
	case Type::OVERLIFETIME:
	case Type::OVERDISTANCE:
	case Type::OVERSPEED:
		if (ImGui::Checkbox(useCurve ? "Disable Opacity Curve" : "Enable Opacity Curve", &useCurve))
			ret = true;

		if (!useCurve)
		{
			ImGui::SameLine();
			if (ImGui::Checkbox("Invert Opacity", &inverted)) ret = true;
		}
		break;
	default:
		break;
	}
	return ret;
}

void RE_PR_Opacity::JsonSerialize(RE_Json* node) const
{
	node->Push("Type", static_cast<uint>(type));

	node->Push("Opacity", opacity);
	node->Push("Inverted", inverted);
	node->Push("useCurve", useCurve);

	curve.JsonSerialize(node->PushJObject("curve"));

	DEL(node);
}

void RE_PR_Opacity::JsonDeserialize(RE_Json* node)
{
	type = static_cast<Type>(node->PullUInt("Type", 0));

	opacity = node->PullFloat("Opacity", 1.f);
	inverted = node->PullBool("Inverted", false);
	useCurve = node->PullBool("useCurve", false);

	curve.JsonDeserialize(node->PullJObject("curve"));

	DEL(node);
}

size_t RE_PR_Opacity::GetBinarySize() const
{
	return sizeof(int)
		+ sizeof(float)
		+ (sizeof(bool) * 2)
		+ curve.GetBinarySize();
}

void RE_PR_Opacity::BinarySerialize(char*& cursor) const
{
	size_t size = sizeof(Type);
	memcpy(cursor, &type, size);
	cursor += size;

	size = sizeof(float);
	memcpy(cursor, &opacity, size);
	cursor += size;

	size = sizeof(bool);
	memcpy(cursor, &inverted, size);
	cursor += size;

	memcpy(cursor, &useCurve, size);
	cursor += size;

	curve.BinarySerialize(cursor);
}

void RE_PR_Opacity::BinaryDeserialize(char*& cursor)
{
	size_t size = sizeof(Type);
	memcpy(&type, cursor, size);
	cursor += size;

	size = sizeof(float);
	memcpy(&opacity, cursor, size);
	cursor += size;

	size = sizeof(bool);
	memcpy(&inverted, cursor, size);
	cursor += size;

	memcpy(&useCurve, cursor, size);
	cursor += size;

	curve.BinaryDeserialize(cursor);
}
