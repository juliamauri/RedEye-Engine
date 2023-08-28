#include "RE_EmissionShape.h"

#include "RE_Memory.h"
#include "Application.h"
#include "RE_Math.h"
#include "RE_Json.h"

#include <ImGui/imgui.h>

bool RE_EmissionShape::IsShaped() const
{
	return type != Type::POINT;
}

math::vec RE_EmissionShape::GetPosition() const
{
	math::vec ret = math::vec::zero;
	switch (type)
	{
	case Type::CIRCLE:
		ret = geo.circle.pos +
			((geo.circle.GetPoint(RE_MATH->RandomF() * RE_Math::pi_x2) - geo.circle.pos) * RE_MATH->RandomF());
	case Type::RING:
		ret = geo.ring.first.GetPoint(RE_MATH->RandomF() * RE_Math::pi_x2) +
			(RE_MATH->RandomNVec() * geo.ring.second);
	case Type::AABB: ret = {
			geo.box.minPoint.x + (RE_MATH->RandomF() * (geo.box.maxPoint.x - geo.box.minPoint.x)),
			geo.box.minPoint.y + (RE_MATH->RandomF() * (geo.box.maxPoint.y - geo.box.minPoint.y)),
			geo.box.minPoint.z + (RE_MATH->RandomF() * (geo.box.maxPoint.z - geo.box.minPoint.z)) };
	case Type::SPHERE:
		ret = geo.sphere.pos +
			((RE_MATH->RandomF() * geo.sphere.r) * RE_MATH->RandomNVec());
	case Type::HOLLOW_SPHERE:
		ret = geo.hollow_sphere.first.pos +
			((geo.hollow_sphere.first.r + (geo.hollow_sphere.second + RE_MATH->RandomFN())) * RE_MATH->RandomNVec());
	default: ret = geo.point;
	}

	return ret;
}

bool RE_EmissionShape::DrawEditor()
{
	bool ret = false;
	int next_shape = static_cast<int>(type);
	if (ImGui::Combo("Emissor Shape", &next_shape, "Point\0Cirle\0Ring\0AABB\0Sphere\0Hollow Sphere\0"))
	{
		switch (type = static_cast<Type>(next_shape))
		{
		case Type::POINT: geo.point = math::vec::zero; break;
		case Type::CIRCLE: geo.circle = math::Circle(math::vec::zero, { 0.f, 1.f, 0.f }, 1.f); break;
		case Type::RING: geo.ring = { math::Circle(math::vec::zero, { 0.f, 1.f, 0.f }, 1.f), 0.1f }; break;
		case Type::AABB: geo.box.SetFromCenterAndSize(math::vec::zero, math::vec::one); break;
		case Type::SPHERE: geo.sphere = math::Sphere(math::vec::zero, 1.f); break;
		case Type::HOLLOW_SPHERE: geo.hollow_sphere = { math::Sphere(math::vec::zero, 1.f), 0.8f }; break;
		default: break;
		}
		ret = true;
	}

	switch (type)
	{
	case Type::POINT:
		if (ImGui::DragFloat3("Starting Pos", geo.point.ptr())) ret = true;
		break;
	case Type::CIRCLE:
	{
		if (ImGui::DragFloat3("Circle Origin", geo.circle.pos.ptr()) ||
			ImGui::DragFloat("Circle Radius", &geo.circle.r, 1.f, 0.f)) ret = true;

		math::float2 angles = geo.circle.normal.ToSphericalCoordinatesNormalized() * RE_Math::rad_to_deg;
		if (ImGui::DragFloat2("Circle Yaw - Pitch", angles.ptr(), 0.1f, -180.f, 180.f))
		{
			angles *= RE_Math::deg_to_rad;
			geo.circle.normal = math::vec::FromSphericalCoordinates(angles.x, angles.y);
			ret = true;
		}
		break;
	}
	case Type::RING:
	{
		if (ImGui::DragFloat3("Ring Origin", geo.ring.first.pos.ptr()) ||
			ImGui::DragFloat("Ring Radius", &geo.ring.first.r, 1.f, 0.f) ||
			ImGui::DragFloat("Ring Inner Radius", &geo.ring.second, 1.f, 0.f, geo.ring.first.r))
			ret = true;

		math::float2 angles = geo.ring.first.normal.ToSphericalCoordinatesNormalized() * RE_Math::rad_to_deg;
		if (ImGui::DragFloat2("Ring Yaw - Pitch", angles.ptr(), 0.1f, -180.f, 180.f))
		{
			angles *= RE_Math::deg_to_rad;
			geo.ring.first.normal = math::vec::FromSphericalCoordinates(angles.x, angles.y);
			ret = true;
		}
		break;
	}
	case Type::AABB:
		if (ImGui::DragFloat3("Box Min ", geo.box.minPoint.ptr()) ||
			ImGui::DragFloat3("Box Max ", geo.box.maxPoint.ptr()))
			ret = true;
		break;
	case Type::SPHERE:
		if (ImGui::DragFloat3("Sphere Origin", geo.sphere.pos.ptr()) ||
			ImGui::DragFloat("Sphere Radius", &geo.sphere.r, 1.f, 0.f))
			ret = true;
		break;
	case Type::HOLLOW_SPHERE:
		if (ImGui::DragFloat3("Hollow Sphere Origin", geo.hollow_sphere.first.pos.ptr()) ||
			ImGui::DragFloat("Hollow Sphere Radius", &geo.hollow_sphere.first.r, 1.f, 0.f) ||
			ImGui::DragFloat("Hollow Sphere Inner Radius", &geo.hollow_sphere.second, 1.f, 0.f, geo.hollow_sphere.first.r))
			ret = true;
		break;
	default: break;
	}

	return ret;
}

void RE_EmissionShape::JsonSerialize(RE_Json* node) const
{
	node->Push("Type", static_cast<uint>(type));

	switch (type)
	{
	case RE_EmissionShape::Type::CIRCLE:
	{
		node->Push("Radius", geo.circle.r);
		node->PushFloatVector("Position", geo.circle.pos);
		node->PushFloatVector("Normal", geo.circle.normal);
		break;
	}
	case RE_EmissionShape::Type::RING:
	{
		node->Push("Inner radius", geo.ring.second);
		node->Push("Radius", geo.ring.first.r);
		node->PushFloatVector("Position", geo.ring.first.pos);
		node->PushFloatVector("Normal", geo.ring.first.normal);
		break;
	}
	case RE_EmissionShape::Type::AABB:
	{
		node->PushFloatVector("Min point", geo.box.minPoint);
		node->PushFloatVector("Max point", geo.box.maxPoint);
		break;
	}
	case RE_EmissionShape::Type::SPHERE:
	{
		node->Push("Radius", geo.sphere.r);
		node->PushFloatVector("Position", geo.sphere.pos);
		break;
	}
	case RE_EmissionShape::Type::HOLLOW_SPHERE:
	{
		node->Push("Inner radius", geo.hollow_sphere.second);
		node->Push("Radius", geo.hollow_sphere.first.r);
		node->PushFloatVector("Position", geo.hollow_sphere.first.pos);
		break;
	}
	default:
	{
		node->PushFloatVector("Position", geo.point);
		break;
	}
	}

	DEL(node);
}

void RE_EmissionShape::JsonDeserialize(RE_Json* node)
{
	type = static_cast<RE_EmissionShape::Type>(node->PullUInt("Type", static_cast<uint>(type)));
	switch (type)
	{
	case RE_EmissionShape::Type::CIRCLE:
		geo.circle.r = node->PullFloat("Radius", geo.circle.r);
		geo.circle.pos = node->PullFloatVector("Position", geo.circle.pos);
		geo.circle.normal = node->PullFloatVector("Normal", geo.circle.normal);
		break;
	case RE_EmissionShape::Type::RING:
		geo.ring.second = node->PullFloat("Inner radius", geo.ring.second);
		geo.ring.first.r = node->PullFloat("Radius", geo.ring.first.r);
		geo.ring.first.pos = node->PullFloatVector("Position", geo.ring.first.pos);
		geo.ring.first.normal = node->PullFloatVector("Normal", geo.ring.first.normal);
		break;
	case RE_EmissionShape::Type::AABB:
		geo.box.minPoint = node->PullFloatVector("Min point", geo.box.minPoint);
		geo.box.maxPoint = node->PullFloatVector("Max point", geo.box.maxPoint);
		break;
	case RE_EmissionShape::Type::SPHERE:
		geo.sphere.r = node->PullFloat("Radius", geo.sphere.r);
		geo.sphere.pos = node->PullFloatVector("Position", geo.sphere.pos);
		break;
	case RE_EmissionShape::Type::HOLLOW_SPHERE:
		geo.hollow_sphere.second = node->PullFloat("Inner radius", geo.hollow_sphere.second);
		geo.hollow_sphere.first.r = node->PullFloat("Radius", geo.hollow_sphere.first.r);
		geo.hollow_sphere.first.pos = node->PullFloatVector("Position", geo.hollow_sphere.first.pos);
		break;
	default:
		geo.point = node->PullFloatVector("Position", geo.point);
		break;
	}

	DEL(node);
}

size_t RE_EmissionShape::GetBinarySize() const
{
	size_t ret = sizeof(ushort);
	switch (type)
	{
	case RE_EmissionShape::Type::CIRCLE: ret += sizeof(float) * 7; break;
	case RE_EmissionShape::Type::RING: ret += sizeof(float) * 8; break;
	case RE_EmissionShape::Type::AABB: ret += sizeof(float) * 6; break;
	case RE_EmissionShape::Type::SPHERE: ret += sizeof(float) * 4; break;
	case RE_EmissionShape::Type::HOLLOW_SPHERE: ret += sizeof(float) * 5; break;
	default: ret += sizeof(float) * 3; break;
	}
	return ret;
}

void RE_EmissionShape::BinarySerialize(char*& cursor) const
{
	size_t size = sizeof(ushort);
	memcpy(cursor, &type, size);
	cursor += size;

	switch (type)
	{
	case Type::CIRCLE:
		size = sizeof(float);
		memcpy(cursor, &geo.circle.r, size);
		cursor += size;
		size *= 3u;
		memcpy(cursor, geo.circle.pos.ptr(), size);
		cursor += size;
		memcpy(cursor, geo.circle.normal.ptr(), size);
		cursor += size;
		break;
	case Type::RING:
		size = sizeof(float);
		memcpy(cursor, &geo.ring.second, size);
		cursor += size;
		memcpy(cursor, &geo.ring.first.r, size);
		cursor += size;
		size *= 3u;
		memcpy(cursor, geo.ring.first.pos.ptr(), size);
		cursor += size;
		memcpy(cursor, geo.ring.first.normal.ptr(), size);
		cursor += size;
		break;
	case Type::AABB:
		size = sizeof(float) * 3;
		memcpy(cursor, geo.box.minPoint.ptr(), size);
		cursor += size;
		memcpy(cursor, geo.box.maxPoint.ptr(), size);
		cursor += size;
		break;
	case Type::SPHERE:
		size = sizeof(float);
		memcpy(cursor, &geo.sphere.r, size);
		cursor += size;
		size *= 3u;
		memcpy(cursor, geo.sphere.pos.ptr(), size);
		cursor += size;
		break;
	case Type::HOLLOW_SPHERE:
		size = sizeof(float);
		memcpy(cursor, &geo.hollow_sphere.second, size);
		cursor += size;
		memcpy(cursor, &geo.hollow_sphere.first.r, size);
		cursor += size;
		size *= 3u;
		memcpy(cursor, geo.hollow_sphere.first.pos.ptr(), size);
		cursor += size;
		break;
	default:
		size = sizeof(float) * 3;
		memcpy(cursor, geo.point.ptr(), size);
		cursor += size;
		break;
	}
}

void RE_EmissionShape::BinaryDeserialize(char*& cursor)
{
	unsigned int size = sizeof(ushort);
	memcpy(&type, cursor, size);
	cursor += size;

	switch (type)
	{
	case RE_EmissionShape::Type::CIRCLE:
		size = sizeof(float);
		memcpy(&geo.circle.r, cursor, size);
		cursor += size;
		size *= 3;
		memcpy(geo.circle.pos.ptr(), cursor, size);
		cursor += size;
		memcpy(geo.circle.normal.ptr(), cursor, size);
		cursor += size;
		break;
	case RE_EmissionShape::Type::RING:
		size = sizeof(float);
		memcpy(&geo.ring.second, cursor, size);
		cursor += size;
		memcpy(&geo.ring.first.r, cursor, size);
		cursor += size;
		size *= 3;
		memcpy(geo.ring.first.pos.ptr(), cursor, size);
		cursor += size;
		memcpy(geo.ring.first.normal.ptr(), cursor, size);
		cursor += size;
		break;
	case RE_EmissionShape::Type::AABB:
		size = sizeof(float) * 3;
		memcpy(geo.box.minPoint.ptr(), cursor, size);
		cursor += size;
		memcpy(geo.box.maxPoint.ptr(), cursor, size);
		cursor += size;
		break;
	case RE_EmissionShape::Type::SPHERE:
		size = sizeof(float);
		memcpy(&geo.sphere.r, cursor, size);
		cursor += size;
		size *= 3;
		memcpy(geo.sphere.pos.ptr(), cursor, size);
		cursor += size;
		break;
	case RE_EmissionShape::Type::HOLLOW_SPHERE:
		size = sizeof(float);
		memcpy(&geo.hollow_sphere.second, cursor, size);
		cursor += size;
		memcpy(&geo.hollow_sphere.first.r, cursor, size);
		cursor += size;
		size *= 3u;
		memcpy(geo.hollow_sphere.first.pos.ptr(), cursor, size);
		cursor += size;
		break;
	default:
		size = sizeof(float) * 3;
		memcpy(geo.point.ptr(), cursor, size);
		cursor += size;
		break;
	}
}
