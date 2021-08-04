#include "RE_EmissionData.h"

#include "RE_Profiler.h"

#include "RE_Memory.h"
#include "Application.h"
#include "ModulePhysics.h"
#include "RE_Math.h"
#include "RE_Particle.h"
#include "RE_Json.h"

#include "ImGui\imgui.h"
#include "ImGuiWidgets/ImGuiCurverEditor/ImGuiCurveEditor.hpp"

bool RE_EmissionInterval::IsActive(float &dt)
{
	switch (type)
	{
	case INTERMITENT:
	{
		if ((time_offset += dt) >= duration[1])
		{
			dt -= (time_offset -= duration[1]);
			is_open = !is_open;
		}

		break;
	}
	case CUSTOM:
	{
		if ((time_offset += dt) >= duration[is_open])
		{
			dt -= (time_offset -= duration[is_open]);
			is_open = !is_open;
		}

		break;
	}
	default:
	{
		is_open = true;
		break;
	}
	}

	return is_open;
}

bool RE_EmissionInterval::DrawEditor(bool& changes)
{
	bool ret = false;
	int tmp = static_cast<int>(type);
	if (ImGui::Combo("Interval", &tmp, "None\0Intermitent\0Custom\0"))
	{
		type = static_cast<RE_EmissionInterval::Type>(tmp);
		is_open = true;
		time_offset = 0.f;
		duration[0] = duration[1] = 1.f;
		changes = ret = true;
	}

	switch (type) {
	case RE_EmissionInterval::INTERMITENT:
	{
		if(ImGui::DragFloat("Interval On", &duration[1], 1.f, 0.f, 10000.f)) changes = true;
		break; 
	}
	case RE_EmissionInterval::CUSTOM:
	{
		if(ImGui::DragFloat("Interval On", &duration[1], 1.f, 0.f, 10000.f)) changes = true;
		if(ImGui::DragFloat("Interval Off", &duration[0], 1.f, 0.f, 10000.f)) changes = true;
		break;
	}
	default: break; }

	return ret;
}

void RE_EmissionInterval::JsonDeserialize(RE_Json* node)
{
	type = static_cast<RE_EmissionInterval::Type>(node->PullInt("Type", static_cast<int>(type)));
	if (type)
	{
		duration[0] = node->PullFloat("Duration 1", duration[0]);
		duration[1] = node->PullFloat("Duration 2", duration[1]);
	}

	DEL(node);
}

void RE_EmissionInterval::JsonSerialize(RE_Json* node) const
{
	node->PushInt("Type", static_cast<int>(type));
	if (type)
	{
		node->PushFloat("Duration 1", duration[0]);
		node->PushFloat("Duration 2", duration[1]);
	}

	DEL(node);
}

void RE_EmissionInterval::BinaryDeserialize(char*& cursor)
{
	unsigned int size = sizeof(int);
	memcpy(&type, cursor, size);
	cursor += size;
	if (type)
	{
		size = sizeof(float) * 2u;
		memcpy(duration, cursor, size);
		cursor += size;
	}
}

void RE_EmissionInterval::BinarySerialize(char*& cursor) const
{
	unsigned int size = sizeof(int);
	memcpy(cursor, &type, size);
	cursor += size;
	if (type)
	{
		size = sizeof(float) * 2u;
		memcpy(cursor, duration, size);
		cursor += size;
	}
}

unsigned int RE_EmissionInterval::GetBinarySize() const
{
	unsigned int ret = sizeof(int);
	if (type) ret += sizeof(float) * 2u;
	return ret;
}

unsigned int RE_EmissionSpawn::CountNewParticles(const float dt)
{
	unsigned int ret = 0u;

	time_offset += dt;

	switch (type)
	{
	case RE_EmissionSpawn::Type::SINGLE:
	{
		if (!has_started)
			ret = static_cast<unsigned int>(particles_spawned);

		break;
	}
	case RE_EmissionSpawn::Type::BURST:
	{
		int mult = static_cast<int>(!has_started);
		while (time_offset >= frequency)
		{
			time_offset -= frequency;
			mult++;
		}

		ret += static_cast<unsigned int>(particles_spawned * mult);

		break;
	}
	case RE_EmissionSpawn::Type::FLOW:
	{
		ret = static_cast<unsigned int>(time_offset * frequency);
		time_offset -= static_cast<float>(ret) / frequency;
		break;
	}
	}

	has_started = true;

	return ret;
}

bool RE_EmissionSpawn::DrawEditor(bool& changes)
{
	bool ret = false;
	int tmp = static_cast<int>(type);
	if (ImGui::Combo("Spawn type", &tmp, "Single\0Burst\0Flow\0"))
	{
		type = static_cast<RE_EmissionSpawn::Type>(tmp);
		has_started = false;
		time_offset = 0.f;
		changes = ret = true;
	}

	switch (type) {
	case RE_EmissionSpawn::Type::SINGLE:
	{
		if (ImGui::DragInt("Particle amount", &particles_spawned, 1.f, 0, 10000)) changes = true;
		break;
	}
	case RE_EmissionSpawn::Type::BURST:
	{
		if(ImGui::DragInt("Particles/burst", &particles_spawned, 1.f, 0, 10000)) changes = true;
		if(ImGui::DragFloat("Period", &frequency, 1.f, 0.0001f, 10000.f)) changes = true;

		break;
	}
	case RE_EmissionSpawn::Type::FLOW:
	{
		if(ImGui::DragFloat("Frecuency", &frequency, 1.f, 0.0001f, 1000.f)) changes = true;
		break;
	}
	}

	return ret;
}

void RE_EmissionSpawn::JsonDeserialize(RE_Json* node)
{
	type = static_cast<RE_EmissionSpawn::Type>(node->PullInt("Type", static_cast<int>(type)));
	switch (type) {
	case RE_EmissionSpawn::Type::SINGLE:
	{
		particles_spawned = node->PullInt("Particles spawned", particles_spawned);
		break;
	}
	case RE_EmissionSpawn::Type::BURST:
	{
		particles_spawned = node->PullInt("Particles spawned", particles_spawned);
		frequency = node->PullFloat("Period", frequency);
		break;
	}
	case RE_EmissionSpawn::Type::FLOW:
	{
		frequency = node->PullFloat("Frequency", frequency);
		break;
	}
	}

	DEL(node);
}

void RE_EmissionSpawn::JsonSerialize(RE_Json* node) const
{
	node->PushInt("Type", static_cast<int>(type));
	switch (type) {
	case RE_EmissionSpawn::Type::SINGLE:
	{
		node->PushInt("Particles spawned", particles_spawned);
		break;
	}
	case RE_EmissionSpawn::Type::BURST:
	{
		node->PushInt("Particles spawned", particles_spawned);
		node->PushFloat("Period", frequency);
		break;
	}
	case RE_EmissionSpawn::Type::FLOW:
	{
		node->PushFloat("Frequency", frequency);
		break;
	}
	}

	DEL(node);
}

void RE_EmissionSpawn::BinaryDeserialize(char*& cursor)
{
	unsigned int size = sizeof(int);
	memcpy(&type, cursor, size);
	cursor += size;
	switch (type) {
	case RE_EmissionSpawn::Type::SINGLE:
	{
		memcpy(&particles_spawned, cursor, size);
		cursor += size;
		break;
	}
	case RE_EmissionSpawn::Type::BURST:
	{
		memcpy(&particles_spawned, cursor, size);
		cursor += size;
		size = sizeof(float);
		memcpy(&frequency, cursor, size);
		cursor += size;
		break;
	}
	case RE_EmissionSpawn::Type::FLOW:
	{
		size = sizeof(float);
		memcpy(&frequency, cursor, size);
		cursor += size;
		break;
	}
	}
}

void RE_EmissionSpawn::BinarySerialize(char*& cursor) const
{
	unsigned int size = sizeof(int);
	memcpy(cursor, &type, size);
	cursor += size;
	switch (type) {
	case RE_EmissionSpawn::Type::SINGLE:
	{
		memcpy(cursor, &particles_spawned, size);
		cursor += size;
		break;
	}
	case RE_EmissionSpawn::Type::BURST:
	{
		memcpy(cursor, &particles_spawned, size);
		cursor += size;
		size = sizeof(float);
		memcpy(cursor, &frequency, size);
		cursor += size;
		break;
	}
	case RE_EmissionSpawn::Type::FLOW:
	{
		size = sizeof(float);
		memcpy(cursor, &frequency, size);
		cursor += size;
		break;
	}
	}
}

unsigned int RE_EmissionSpawn::GetBinarySize() const
{
	unsigned int ret = sizeof(int);
	switch (type) {
	case RE_EmissionSpawn::Type::SINGLE: ret *= 2u; break;
	case RE_EmissionSpawn::Type::BURST: ret *= 2u; ret += sizeof(float); break;
	case RE_EmissionSpawn::Type::FLOW: ret += sizeof(float); break; }
	return ret;
}

math::vec RE_EmissionShape::GetPosition() const
{
	switch (type) {
	case CIRCLE: return ((geo.circle.GetPoint(RE_MATH->RandomF() * RE_Math::pi_x2) - geo.circle.pos) * RE_MATH->RandomF()) + geo.circle.pos;
	case RING: return geo.ring.first.GetPoint(RE_MATH->RandomF() * RE_Math::pi_x2) + (RE_MATH->RandomNVec() * geo.ring.second);
	case AABB: return { 
			geo.box.minPoint.x + (RE_MATH->RandomF() * (geo.box.maxPoint.x - geo.box.minPoint.x)),
			geo.box.minPoint.y + (RE_MATH->RandomF() * (geo.box.maxPoint.y - geo.box.minPoint.y)),
			geo.box.minPoint.z + (RE_MATH->RandomF() * (geo.box.maxPoint.z - geo.box.minPoint.z)) };
	case SPHERE: return geo.sphere.pos + ((RE_MATH->RandomF() * geo.sphere.r) * RE_MATH->RandomNVec());
	case HOLLOW_SPHERE: return geo.hollow_sphere.first.pos + ((geo.hollow_sphere.first.r + (geo.hollow_sphere.second + RE_MATH->RandomFN())) * RE_MATH->RandomNVec());
	default: return geo.point; }
}

bool RE_EmissionShape::DrawEditor()
{
	bool ret = false;
	int next_shape = static_cast<int>(type);
	if (ImGui::Combo("Emissor Shape", &next_shape, "Point\0Cirle\0Ring\0AABB\0Sphere\0Hollow Sphere\0"))
	{
		switch (type = static_cast<Type>(next_shape)) {
		case RE_EmissionShape::POINT: geo.point = math::vec::zero; break;
		case RE_EmissionShape::CIRCLE: geo.circle = math::Circle(math::vec::zero, { 0.f, 1.f, 0.f }, 1.f); break;
		case RE_EmissionShape::RING: geo.ring = { math::Circle(math::vec::zero, { 0.f, 1.f, 0.f }, 1.f), 0.1f }; break;
		case RE_EmissionShape::AABB: geo.box.SetFromCenterAndSize(math::vec::zero, math::vec::one); break;
		case RE_EmissionShape::SPHERE: geo.sphere = math::Sphere(math::vec::zero, 1.f); break;
		case RE_EmissionShape::HOLLOW_SPHERE: geo.hollow_sphere = { math::Sphere(math::vec::zero, 1.f), 0.8f }; break; }
		ret = true;
	}

	switch (type)
	{
	case RE_EmissionShape::POINT:
	{
		if(ImGui::DragFloat3("Starting Pos", geo.point.ptr())) ret = true;
		break;
	}
	case RE_EmissionShape::CIRCLE:
	{
		if(ImGui::DragFloat3("Circle Origin", geo.circle.pos.ptr())) ret = true;
		if(ImGui::DragFloat("Circle Radius", &geo.circle.r, 1.f, 0.f)) ret = true;

		math::float2 angles = geo.circle.normal.ToSphericalCoordinatesNormalized() * RE_Math::rad_to_deg;
		if (ImGui::DragFloat2("Circle Yaw - Pitch", angles.ptr(), 0.1f, -180.f, 180.f))
		{
			angles *= RE_Math::deg_to_rad;
			geo.circle.normal = math::vec::FromSphericalCoordinates(angles.x, angles.y);
			ret = true;
		}
		break;
	}
	case RE_EmissionShape::RING:
	{
		if(ImGui::DragFloat3("Ring Origin", geo.ring.first.pos.ptr())) ret = true;
		if(ImGui::DragFloat("Ring Radius", &geo.ring.first.r, 1.f, 0.f)) ret = true;
		if(ImGui::DragFloat("Ring Inner Radius", &geo.ring.second, 1.f, 0.f, geo.ring.first.r)) ret = true;

		math::float2 angles = geo.ring.first.normal.ToSphericalCoordinatesNormalized() * RE_Math::rad_to_deg;
		if (ImGui::DragFloat2("Ring Yaw - Pitch", angles.ptr(), 0.1f, -180.f, 180.f))
		{
			angles *= RE_Math::deg_to_rad;
			geo.ring.first.normal = math::vec::FromSphericalCoordinates(angles.x, angles.y);
			ret = true;
		}
		break;
	}
	case RE_EmissionShape::AABB:
	{
		if(ImGui::DragFloat3("Box Min ", geo.box.minPoint.ptr())) ret = true;
		if(ImGui::DragFloat3("Box Max ", geo.box.maxPoint.ptr())) ret = true;
		break;
	}
	case RE_EmissionShape::SPHERE:
	{
		if(ImGui::DragFloat3("Sphere Origin", geo.sphere.pos.ptr())) ret = true;
		if(ImGui::DragFloat("Sphere Radius", &geo.sphere.r, 1.f, 0.f)) ret = true;
		break;
	}
	case RE_EmissionShape::HOLLOW_SPHERE:
	{
		if(ImGui::DragFloat3("Hollow Sphere Origin", geo.hollow_sphere.first.pos.ptr())) ret = true;
		if(ImGui::DragFloat("Hollow Sphere Radius", &geo.hollow_sphere.first.r, 1.f, 0.f)) ret = true;
		if(ImGui::DragFloat("Hollow Sphere Inner Radius", &geo.hollow_sphere.second, 1.f, 0.f, geo.hollow_sphere.first.r)) ret = true;
		break;
	}
	}

	return ret;
}

void RE_EmissionShape::JsonDeserialize(RE_Json* node)
{
	type = static_cast<RE_EmissionShape::Type>(node->PullInt("Type", static_cast<int>(type)));
	switch (type) {
	case RE_EmissionShape::Type::CIRCLE:
	{
		geo.circle.r = node->PullFloat("Radius", geo.circle.r);
		geo.circle.pos = node->PullFloatVector("Position", geo.circle.pos);
		geo.circle.normal = node->PullFloatVector("Normal", geo.circle.normal);
		break;
	}
	case RE_EmissionShape::Type::RING:
	{
		geo.ring.second = node->PullFloat("Inner radius", geo.ring.second);
		geo.ring.first.r = node->PullFloat("Radius", geo.ring.first.r);
		geo.ring.first.pos = node->PullFloatVector("Position", geo.ring.first.pos);
		geo.ring.first.normal = node->PullFloatVector("Normal", geo.ring.first.normal);
		break;
	}
	case RE_EmissionShape::Type::AABB:
	{
		geo.box.minPoint = node->PullFloatVector("Min point", geo.box.minPoint);
		geo.box.maxPoint = node->PullFloatVector("Max point", geo.box.maxPoint);
		break;
	}
	case RE_EmissionShape::Type::SPHERE:
	{
		geo.sphere.r = node->PullFloat("Radius", geo.sphere.r);
		geo.sphere.pos = node->PullFloatVector("Position", geo.sphere.pos);
		break;
	}
	case RE_EmissionShape::Type::HOLLOW_SPHERE:
	{
		geo.hollow_sphere.second = node->PullFloat("Inner radius", geo.hollow_sphere.second);
		geo.hollow_sphere.first.r = node->PullFloat("Radius", geo.hollow_sphere.first.r);
		geo.hollow_sphere.first.pos = node->PullFloatVector("Position", geo.hollow_sphere.first.pos);
		break;
	}
	default:
	{
		geo.point = node->PullFloatVector("Position", geo.point);
		break;
	}
	}

	DEL(node);
}

void RE_EmissionShape::JsonSerialize(RE_Json* node) const
{
	node->PushInt("Type", static_cast<int>(type));
	switch (type) {
	case RE_EmissionShape::Type::CIRCLE:
	{
		node->PushFloat("Radius", geo.circle.r);
		node->PushFloatVector("Position", geo.circle.pos);
		node->PushFloatVector("Normal", geo.circle.normal);
		break;
	}
	case RE_EmissionShape::Type::RING:
	{
		node->PushFloat("Inner radius", geo.ring.second);
		node->PushFloat("Radius", geo.ring.first.r);
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
		node->PushFloat("Radius", geo.sphere.r);
		node->PushFloatVector("Position", geo.sphere.pos);
		break;
	}
	case RE_EmissionShape::Type::HOLLOW_SPHERE:
	{
		node->PushFloat("Inner radius", geo.hollow_sphere.second);
		node->PushFloat("Radius", geo.hollow_sphere.first.r);
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

void RE_EmissionShape::BinaryDeserialize(char*& cursor)
{
	unsigned int size = sizeof(int);
	memcpy(&type, cursor, size);
	cursor += size;
	switch (type) {
	case RE_EmissionShape::Type::CIRCLE:
	{
		size = sizeof(float);
		memcpy(&geo.circle.r, cursor, size);
		cursor += size;
		size *= 3u;
		memcpy(geo.circle.pos.ptr(), cursor, size);
		cursor += size;
		memcpy(geo.circle.normal.ptr(), cursor, size);
		cursor += size;

		break;
	}
	case RE_EmissionShape::Type::RING:
	{
		size = sizeof(float);
		memcpy(&geo.ring.second, cursor, size);
		cursor += size;
		memcpy(&geo.ring.first.r, cursor, size);
		cursor += size;
		size *= 3u;
		memcpy(geo.ring.first.pos.ptr(), cursor, size);
		cursor += size;
		memcpy(geo.ring.first.normal.ptr(), cursor, size);
		cursor += size;

		break;
	}
	case RE_EmissionShape::Type::AABB:
	{
		size = sizeof(float) * 3u;
		memcpy(geo.box.minPoint.ptr(), cursor, size);
		cursor += size;
		memcpy(geo.box.maxPoint.ptr(), cursor, size);
		cursor += size;
		break;
	}
	case RE_EmissionShape::Type::SPHERE:
	{
		size = sizeof(float);
		memcpy(&geo.sphere.r, cursor, size);
		cursor += size;
		size *= 3u;
		memcpy(geo.sphere.pos.ptr(), cursor, size);
		cursor += size;

		break;
	}
	case RE_EmissionShape::Type::HOLLOW_SPHERE:
	{
		size = sizeof(float);
		memcpy(&geo.hollow_sphere.second, cursor, size);
		cursor += size;
		memcpy(&geo.hollow_sphere.first.r, cursor, size);
		cursor += size;
		size *= 3u;
		memcpy(geo.hollow_sphere.first.pos.ptr(), cursor, size);
		cursor += size;
		break;
	}
	default:
	{
		size = sizeof(float) * 3u;
		memcpy(geo.point.ptr(), cursor, size);
		cursor += size;
		break;
	}
	}
}

void RE_EmissionShape::BinarySerialize(char*& cursor) const
{
	unsigned int size = sizeof(int);
	memcpy(cursor, &type, size);
	cursor += size;
	switch (type) {
	case RE_EmissionShape::Type::CIRCLE:
	{
		size = sizeof(float);
		memcpy(cursor, &geo.circle.r, size);
		cursor += size;
		size *= 3u;
		memcpy(cursor, geo.circle.pos.ptr(), size);
		cursor += size;
		memcpy(cursor, geo.circle.normal.ptr(), size);
		cursor += size;

		break;
	}
	case RE_EmissionShape::Type::RING:
	{
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
	}
	case RE_EmissionShape::Type::AABB:
	{
		size = sizeof(float) * 3u;
		memcpy(cursor, geo.box.minPoint.ptr(), size);
		cursor += size;
		memcpy(cursor, geo.box.maxPoint.ptr(), size);
		cursor += size;
		break;
	}
	case RE_EmissionShape::Type::SPHERE:
	{
		size = sizeof(float);
		memcpy(cursor, &geo.sphere.r, size);
		cursor += size;
		size *= 3u;
		memcpy(cursor, geo.sphere.pos.ptr(), size);
		cursor += size;

		break;
	}
	case RE_EmissionShape::Type::HOLLOW_SPHERE:
	{
		size = sizeof(float);
		memcpy(cursor, &geo.hollow_sphere.second, size);
		cursor += size;
		memcpy(cursor, &geo.hollow_sphere.first.r, size);
		cursor += size;
		size *= 3u;
		memcpy(cursor, geo.hollow_sphere.first.pos.ptr(), size);
		cursor += size;
		break;
	}
	default:
	{
		size = sizeof(float) * 3u;
		memcpy(cursor, geo.point.ptr(), size);
		cursor += size;
		break;
	}
	}
}

unsigned int RE_EmissionShape::GetBinarySize() const
{
	unsigned int ret = sizeof(int);
	switch (type) {
	case RE_EmissionShape::Type::CIRCLE: ret += sizeof(float) * 7u; break;
	case RE_EmissionShape::Type::RING: ret += sizeof(float) * 8u; break;
	case RE_EmissionShape::Type::AABB: ret += sizeof(float) * 6u; break;
	case RE_EmissionShape::Type::SPHERE: ret += sizeof(float) * 4u; break;
	case RE_EmissionShape::Type::HOLLOW_SPHERE: ret += sizeof(float) * 5u; break;
	default: ret += sizeof(float) * 3u; break; }
	return ret;
}

math::vec RE_EmissionVector::GetValue() const
{
	switch (type) {
	case VALUE: return val;
	case RANGEX: return { val.x + (RE_MATH->RandomFN() * margin.x), val.y, val.z};
	case RANGEY: return { val.x, val.y + (RE_MATH->RandomFN() * margin.y), val.z };
	case RANGEZ: return { val.x, val.y, val.z + (RE_MATH->RandomFN() * margin.z) };
	case RANGEXY: return { val.x + (RE_MATH->RandomFN() * margin.x), val.y + (RE_MATH->RandomFN() * margin.y), val.z };
	case RANGEXZ: return { val.x + (RE_MATH->RandomFN() * margin.x), val.y, val.z + (RE_MATH->RandomFN() * margin.z) };
	case RANGEYZ: return { val.x, val.y + (RE_MATH->RandomFN() * margin.y), val.z + (RE_MATH->RandomFN() * margin.z) };
	case RANGEXYZ: return { val.x + (RE_MATH->RandomFN() * margin.x), val.y + (RE_MATH->RandomFN() * margin.y), val.z + (RE_MATH->RandomFN() * margin.z) };
	default: return math::vec::zero; }
}

bool RE_EmissionVector::DrawEditor(const char* name)
{
	bool ret = false;
	const eastl::string tmp(name);
	int next_type = static_cast<int>(type);
	if (ImGui::Combo((tmp + " type").c_str(), &next_type, "None\0Value\0Range X\0Range Y\0Range Z\0Range XY\0Range XZ\0Range YZ\0Range XYZ\0")) {
		type = static_cast<Type>(next_type);
		ret = true;
	}

	switch (type) {
	case RE_EmissionVector::VALUE: ImGui::DragFloat3(name, val.ptr()); break;
	case RE_EmissionVector::RANGEX:
	{
		if(ImGui::DragFloat3(name, val.ptr())) ret = true;
		if(ImGui::DragFloat((tmp + " X Margin").c_str(), &margin.x)) ret = true;
		break; 
	}
	case RE_EmissionVector::RANGEY:
	{
		if(ImGui::DragFloat3(name, val.ptr())) ret = true;
		if(ImGui::DragFloat((tmp + " Y Margin").c_str(), &margin.y)) ret = true;
		break; 
	}
	case RE_EmissionVector::RANGEZ:
	{
		if(ImGui::DragFloat3(name, val.ptr())) ret = true;
		if(ImGui::DragFloat((tmp + " Z Margin").c_str(), &margin.z)) ret = true;
		break; 
	}
	case RE_EmissionVector::RANGEXY:
	{
		if(ImGui::DragFloat3(name, val.ptr())) ret = true;
		if(ImGui::DragFloat2((tmp + " XY Margin").c_str(), &margin.x)) ret = true;
		if(ImGui::DragFloat((tmp + " Y Margin").c_str(), &margin.y)) ret = true;
		break; 
	}
	case RE_EmissionVector::RANGEXZ:
	{
		if(ImGui::DragFloat3(name, val.ptr())) ret = true;
		float xz[2] = { margin.x, margin.z };
		if (ImGui::DragFloat2((tmp + " XZ Margin").c_str(), xz))
		{
			margin.x = xz[0];
			margin.z = xz[1];
			ret = true;
		}
		break; 
	}
	case RE_EmissionVector::RANGEYZ:
	{
		if(ImGui::DragFloat3(name, val.ptr())) ret = true;
		if(ImGui::DragFloat2((tmp + " YZ Margin").c_str(), margin.ptr() + 1)) ret = true;
		break; 
	}
	case RE_EmissionVector::RANGEXYZ:
	{
		if(ImGui::DragFloat3(name, val.ptr())) ret = true;
		if(ImGui::DragFloat3((tmp + " Margin").c_str(), margin.ptr())) ret = true;
		break;
	}
	default: break; }
	return ret;
}

void RE_EmissionVector::JsonDeserialize(RE_Json* node)
{
	type = static_cast<RE_EmissionVector::Type>(node->PullInt("Type", static_cast<int>(type)));
	switch (type) {
	case RE_EmissionVector::Type::VALUE:
	{
		val = node->PullFloatVector("Value", val);
		break;
	}
	case RE_EmissionVector::Type::RANGEX:
	{
		margin.x = node->PullFloat("Margin X", margin.x);
		val = node->PullFloatVector("Value", val);
		break;
	}
	case RE_EmissionVector::Type::RANGEY:
	{
		margin.y = node->PullFloat("Margin Y", margin.y);
		val = node->PullFloatVector("Value", val);
		break;
	}
	case RE_EmissionVector::Type::RANGEZ:
	{
		margin.z = node->PullFloat("Margin Z", margin.z);
		val = node->PullFloatVector("Value", val);
		break;
	}
	case RE_EmissionVector::Type::RANGEXY:
	{
		margin.x = node->PullFloat("Margin X", margin.x);
		margin.y = node->PullFloat("Margin Y", margin.y);
		val = node->PullFloatVector("Value", val);
		break;
	}
	case RE_EmissionVector::Type::RANGEXZ:
	{
		margin.x = node->PullFloat("Margin X", margin.x);
		margin.z = node->PullFloat("Margin Z", margin.z);
		val = node->PullFloatVector("Value", val);
		break;
	}
	case RE_EmissionVector::Type::RANGEYZ:
	{
		margin.y = node->PullFloat("Margin Y", margin.y);
		margin.z = node->PullFloat("Margin Z", margin.z);
		val = node->PullFloatVector("Value", val);
		break;
	}
	case RE_EmissionVector::Type::RANGEXYZ:
	{
		margin = node->PullFloatVector("Margin", margin);
		val = node->PullFloatVector("Value", val);
		break;
	}
	default: break; }

	DEL(node);
}

void RE_EmissionVector::JsonSerialize(RE_Json* node) const
{
	node->PushInt("Type", static_cast<int>(type));
	switch (type) {
	case RE_EmissionVector::Type::VALUE:
	{
		node->PushFloatVector("Value", val);
		break;
	}
	case RE_EmissionVector::Type::RANGEX:
	{
		node->PushFloat("Margin X", margin.x);
		node->PushFloatVector("Value", val);
		break;
	}
	case RE_EmissionVector::Type::RANGEY:
	{
		node->PushFloat("Margin Y", margin.y);
		node->PushFloatVector("Value", val);
		break;
	}
	case RE_EmissionVector::Type::RANGEZ:
	{
		node->PushFloat("Margin Z", margin.z);
		node->PushFloatVector("Value", val);
		break;
	}
	case RE_EmissionVector::Type::RANGEXY:
	{
		node->PushFloat("Margin X", margin.x);
		node->PushFloat("Margin Y", margin.y);
		node->PushFloatVector("Value", val);
		break;
	}
	case RE_EmissionVector::Type::RANGEXZ:
	{
		node->PushFloat("Margin X", margin.x);
		node->PushFloat("Margin Z", margin.z);
		node->PushFloatVector("Value", val);
		break;
	}
	case RE_EmissionVector::Type::RANGEYZ:
	{
		node->PushFloat("Margin Y", margin.y);
		node->PushFloat("Margin Z", margin.z);
		node->PushFloatVector("Value", val);
		break;
	}
	case RE_EmissionVector::Type::RANGEXYZ:
	{
		node->PushFloatVector("Margin", margin);
		node->PushFloatVector("Value", val);
		break;
	}
	default: break; }

	DEL(node);
}

void RE_EmissionVector::BinaryDeserialize(char*& cursor)
{
	unsigned int size = sizeof(int);
	memcpy(&type, cursor, size);
	cursor += size;
	switch (type) {
	case RE_EmissionVector::Type::VALUE:
	{
		size = sizeof(float) * 3u;
		memcpy(val.ptr(), cursor, size);
		cursor += size;
		break;
	}
	case RE_EmissionVector::Type::RANGEX:
	{
		size = sizeof(float);
		memcpy(&margin.x, cursor, size);
		cursor += size;
		size *= 3u;
		memcpy(val.ptr(), cursor, size);
		cursor += size;
		break;
	}
	case RE_EmissionVector::Type::RANGEY:
	{
		size = sizeof(float);
		memcpy(&margin.y, cursor, size);
		cursor += size;
		size *= 3u;
		memcpy(val.ptr(), cursor, size);
		cursor += size;
		break;
	}
	case RE_EmissionVector::Type::RANGEZ:
	{
		size = sizeof(float);
		memcpy(&margin.z, cursor, size);
		cursor += size;
		size *= 3u;
		memcpy(val.ptr(), cursor, size);
		cursor += size;
		break;
	}
	case RE_EmissionVector::Type::RANGEXY:
	{
		size = sizeof(float);
		memcpy(&margin.x, cursor, size);
		cursor += size;
		memcpy(&margin.y, cursor, size);
		cursor += size;
		size *= 3u;
		memcpy(val.ptr(), cursor, size);
		cursor += size;
		break;
	}
	case RE_EmissionVector::Type::RANGEXZ:
	{
		size = sizeof(float);
		memcpy(&margin.x, cursor, size);
		cursor += size;
		memcpy(&margin.z, cursor, size);
		cursor += size;
		size *= 3u;
		memcpy(val.ptr(), cursor, size);
		cursor += size;
		break;
	}
	case RE_EmissionVector::Type::RANGEYZ:
	{
		size = sizeof(float);
		memcpy(&margin.y, cursor, size);
		cursor += size;
		memcpy(&margin.z, cursor, size);
		cursor += size;
		size *= 3u;
		memcpy(val.ptr(), cursor, size);
		cursor += size;
		break;
	}
	case RE_EmissionVector::Type::RANGEXYZ:
	{
		size = sizeof(float) * 3u;
		memcpy(margin.ptr(), cursor, size);
		cursor += size;
		memcpy(val.ptr(), cursor, size);
		cursor += size;
		break;
	}
	default: break;
	}
}

void RE_EmissionVector::BinarySerialize(char*& cursor) const
{
	unsigned int size = sizeof(int);
	memcpy(cursor, &type, size);
	cursor += size;
	switch (type) {
	case RE_EmissionVector::Type::VALUE:
	{
		size = sizeof(float) * 3u;
		memcpy(cursor, val.ptr(), size);
		cursor += size;
		break;
	}
	case RE_EmissionVector::Type::RANGEX:
	{
		size = sizeof(float);
		memcpy(cursor, &margin.x, size);
		cursor += size;
		size *= 3u;
		memcpy(cursor, val.ptr(), size);
		cursor += size;
		break;
	}
	case RE_EmissionVector::Type::RANGEY:
	{
		size = sizeof(float);
		memcpy(cursor, &margin.y, size);
		cursor += size;
		size *= 3u;
		memcpy(cursor, val.ptr(), size);
		cursor += size;
		break;
	}
	case RE_EmissionVector::Type::RANGEZ:
	{
		size = sizeof(float);
		memcpy(cursor, &margin.z, size);
		cursor += size;
		size *= 3u;
		memcpy(cursor, val.ptr(), size);
		cursor += size;
		break;
	}
	case RE_EmissionVector::Type::RANGEXY:
	{
		size = sizeof(float);
		memcpy(cursor, &margin.x, size);
		cursor += size;
		memcpy(cursor, &margin.y, size);
		cursor += size;
		size *= 3u;
		memcpy(cursor, val.ptr(), size);
		cursor += size;
		break;
	}
	case RE_EmissionVector::Type::RANGEXZ:
	{
		size = sizeof(float);
		memcpy(cursor, &margin.x, size);
		cursor += size;
		memcpy(cursor, &margin.z, size);
		cursor += size;
		size *= 3u;
		memcpy(cursor, val.ptr(), size);
		cursor += size;
		break;
	}
	case RE_EmissionVector::Type::RANGEYZ:
	{
		size = sizeof(float);
		memcpy(cursor, &margin.y, size);
		cursor += size;
		memcpy(cursor, &margin.z, size);
		cursor += size;
		size *= 3u;
		memcpy(cursor, val.ptr(), size);
		cursor += size;
		break;
	}
	case RE_EmissionVector::Type::RANGEXYZ:
	{
		size = sizeof(float) * 3u;
		memcpy(cursor, margin.ptr(), size);
		cursor += size;
		memcpy(cursor, val.ptr(), size);
		cursor += size;
		break;
	}
	default: break;
	}
}

unsigned int RE_EmissionVector::GetBinarySize() const
{
	unsigned int ret = sizeof(float);
	switch (type) {
	case RE_EmissionVector::Type::VALUE: ret *= 3u; break;
	case RE_EmissionVector::Type::RANGEX: ret *= 4u; break;
	case RE_EmissionVector::Type::RANGEY: ret *= 4u; break;
	case RE_EmissionVector::Type::RANGEZ: ret *= 4u; break;
	case RE_EmissionVector::Type::RANGEXY: ret *= 5u; break;
	case RE_EmissionVector::Type::RANGEXZ: ret *= 5u; break;
	case RE_EmissionVector::Type::RANGEYZ: ret *= 5u; break;
	case RE_EmissionVector::Type::RANGEXYZ: ret *= 6u; break;
	default: break; }
	ret += sizeof(int);
	return ret;
}

float RE_EmissionSingleValue::GetValue() const
{
	switch (type) {
	case VALUE: return val;
	case RANGE: return val + (RE_MATH->RandomFN() * margin);
	default: return 0.f; }
}

float RE_EmissionSingleValue::GetMin() const
{
	switch (type) {
	case VALUE: return val;
	case RANGE: return val - margin;
	default: return 0.f; }
}

float RE_EmissionSingleValue::GetMax() const
{
	switch (type) {
	case VALUE: return val;
	case RANGE: return val + margin;
	default: return 0.f; }
}

bool RE_EmissionSingleValue::DrawEditor(const char* name)
{
	bool ret = false;
	const eastl::string tmp(name);
	int next_type = static_cast<int>(type);
	if (ImGui::Combo((tmp + " type").c_str(), &next_type, "None\0Value\0Range\0")) {
		type = static_cast<Type>(next_type);
		ret = true;
	}

	switch (type) {
	case RE_EmissionSingleValue::NONE: break;
	case RE_EmissionSingleValue::VALUE: if (ImGui::DragFloat(name, &val)) ret = true; break;
	case RE_EmissionSingleValue::RANGE: {
		if(ImGui::DragFloat(name, &val)) ret = true;
		if(ImGui::DragFloat((tmp + " Margin").c_str(), &margin, 0.01f, 0.f)) ret = true;
		break;
	}
	}
	return ret;
}

void RE_EmissionSingleValue::JsonDeserialize(RE_Json* node)
{
	type = static_cast<RE_EmissionSingleValue::Type>(node->PullInt("Type", static_cast<int>(type)));
	switch (type) {
	case RE_EmissionSingleValue::Type::VALUE:
	{
		val = node->PullFloat("Value", val);
		break;
	}
	case RE_EmissionSingleValue::Type::RANGE:
	{
		val = node->PullFloat("Value", val);
		margin = node->PullFloat("Margin", margin);
		break;
	}
	default: break; }

	DEL(node);
}

void RE_EmissionSingleValue::JsonSerialize(RE_Json* node) const
{
	node->PushInt("Type", static_cast<int>(type));
	switch (type) {
	case RE_EmissionSingleValue::Type::VALUE:
	{
		node->PushFloat("Value", val);
		break;
	}
	case RE_EmissionSingleValue::Type::RANGE:
	{
		node->PushFloat("Value", val);
		node->PushFloat("Margin", margin);
		break;
	}
	default: break; }

	DEL(node);
}

void RE_EmissionSingleValue::BinaryDeserialize(char*& cursor)
{
	unsigned int size = sizeof(int);
	memcpy(&type, cursor, size);
	cursor += size;
	switch (type) {
	case RE_EmissionSingleValue::Type::VALUE:
	{
		size = sizeof(float);
		memcpy(&val, cursor, size);
		cursor += size;
		break;
	}
	case RE_EmissionSingleValue::Type::RANGE:
	{
		size = sizeof(float);
		memcpy(&val, cursor, size);
		cursor += size;
		memcpy(&margin, cursor, size);
		cursor += size;
		break;
	}
	default: break;
	}
}

void RE_EmissionSingleValue::BinarySerialize(char*& cursor) const
{
	unsigned int size = sizeof(int);
	memcpy(cursor, &type, size);
	cursor += size;
	switch (type) {
	case RE_EmissionSingleValue::Type::VALUE:
	{
		size = sizeof(float);
		memcpy(cursor, &val, size);
		cursor += size;
		break;
	}
	case RE_EmissionSingleValue::Type::RANGE:
	{
		size = sizeof(float);
		memcpy(cursor, &val, size);
		cursor += size;
		memcpy(cursor, &margin, size);
		cursor += size;
		break;
	}
	default: break; }
}

unsigned int RE_EmissionSingleValue::GetBinarySize() const
{
	unsigned int ret = sizeof(int);
	switch (type) {
	case RE_EmissionSingleValue::Type::VALUE: ret += sizeof(float); break;
	case RE_EmissionSingleValue::Type::RANGE: ret += sizeof(float) * 2u; break;
	default: break; }
	return ret;
}

math::vec RE_EmissionExternalForces::GetAcceleration() const
{
	switch (type) {
	case RE_EmissionExternalForces::GRAVITY: return math::vec(0.f, gravity, 0.f);
	case RE_EmissionExternalForces::WIND: return wind;
	case RE_EmissionExternalForces::WIND_GRAVITY: return math::vec(wind.x, wind.y + gravity, wind.z);
	default: return math::vec::zero; }
}

bool RE_EmissionExternalForces::DrawEditor()
{
	bool ret = false;

	int next_type = static_cast<int>(type);
	if (ImGui::Combo("External Forces", &next_type, "None\0Gravity\0Wind\0Gravity + Wind\0")) {
		type = static_cast<Type>(next_type);
		ret = true;
	}

	switch (type) {
	case RE_EmissionExternalForces::NONE: break;
	case RE_EmissionExternalForces::GRAVITY: if (ImGui::DragFloat("Gravity", &gravity)) ret = true; break;
	case RE_EmissionExternalForces::WIND: if (ImGui::DragFloat3("Wind", wind.ptr())) ret = true; break;
	case RE_EmissionExternalForces::WIND_GRAVITY: {
		if(ImGui::DragFloat("Gravity", &gravity)) ret = true;
		if(ImGui::DragFloat3("Wind", wind.ptr())) ret = true;
		break; 
	}
	}

	return ret;
}

void RE_EmissionExternalForces::JsonDeserialize(RE_Json* node)
{
	type = static_cast<RE_EmissionExternalForces::Type>(node->PullInt("Type", static_cast<int>(type)));
	switch (type) {
	case RE_EmissionExternalForces::GRAVITY:
	{
		gravity = node->PullFloat("Gravity", gravity);
		break;
	}
	case RE_EmissionExternalForces::WIND:
	{
		wind = node->PullFloatVector("Wind", wind);
		break;
	}
	case RE_EmissionExternalForces::WIND_GRAVITY:
	{
		gravity = node->PullFloat("Gravity", gravity);
		wind = node->PullFloatVector("Wind", wind);
		break;
	}
	default: break; }

	DEL(node);
}

void RE_EmissionExternalForces::JsonSerialize(RE_Json* node) const
{
	node->PushInt("Type", static_cast<int>(type));
	switch (type) {
	case RE_EmissionExternalForces::GRAVITY:
	{
		node->PushFloat("Gravity", gravity);
		break;
	}
	case RE_EmissionExternalForces::WIND:
	{
		node->PushFloatVector("Wind", wind);
		break;
	}
	case RE_EmissionExternalForces::WIND_GRAVITY:
	{
		node->PushFloat("Gravity", gravity);
		node->PushFloatVector("Wind", wind);
		break;
	}
	default: break; }

	DEL(node);
}

void RE_EmissionExternalForces::BinaryDeserialize(char*& cursor)
{
	unsigned int size = sizeof(int);
	memcpy(&type, cursor, size);
	cursor += size;
	switch (type) {
	case RE_EmissionExternalForces::GRAVITY:
	{
		size = sizeof(float);
		memcpy(&gravity, cursor, size);
		cursor += size;
		break;
	}
	case RE_EmissionExternalForces::WIND:
	{
		size = sizeof(float) * 3u;
		memcpy(wind.ptr(), cursor, size);
		cursor += size;
		break;
	}
	case RE_EmissionExternalForces::WIND_GRAVITY:
	{
		size = sizeof(float);
		memcpy(&gravity, cursor, size);
		cursor += size;
		size *= 3u;
		memcpy(wind.ptr(), cursor, size);
		cursor += size;
		break;
	}
	default: break;
	}
}

void RE_EmissionExternalForces::BinarySerialize(char*& cursor) const
{
	unsigned int size = sizeof(int);
	memcpy(cursor, &type, size);
	cursor += size;
	switch (type) {
	case RE_EmissionExternalForces::GRAVITY:
	{
		size = sizeof(float);
		memcpy(cursor, &gravity, size);
		cursor += size;
		break;
	}
	case RE_EmissionExternalForces::WIND:
	{
		size = sizeof(float) * 3u;
		memcpy(cursor, wind.ptr(), size);
		cursor += size;
		break;
	}
	case RE_EmissionExternalForces::WIND_GRAVITY:
	{
		size = sizeof(float);
		memcpy(cursor, &gravity, size);
		cursor += size;
		size *= 3u;
		memcpy(cursor, wind.ptr(), size);
		cursor += size;
		break;
	}
	default: break; }
}

unsigned int RE_EmissionExternalForces::GetBinarySize() const
{
	unsigned int ret = sizeof(int);
	switch (type) {
	case RE_EmissionExternalForces::Type::GRAVITY: ret += sizeof(float); break;
	case RE_EmissionExternalForces::Type::WIND: ret += sizeof(float) * 3u; break;
	case RE_EmissionExternalForces::Type::WIND_GRAVITY: ret += sizeof(float) * 4u; break;
	default: break; }
	return ret;
}

bool RE_EmissionBoundary::PointCollision(RE_Particle& p) const
{
	RE_PROFILE(PROF_ParticleBoundPCol, PROF_ParticleBoundary);
	switch (type)
	{
	case Type::PLANE:
	{
		// Check if particle intersects or has passed plane
		float dist_to_plane = geo.plane.SignedDistance(p.position);
		if (dist_to_plane <= 0.f)
		{
#ifdef PARTICLE_PHYSICS_TEST
			ProfilingTimer::p_col_boundary++;
#endif // PARTICLE_PHYSICS_TEST

			if (effect == RE_EmissionBoundary::KILL) return false;

			// Resolve intersection
			const math::vec norm_speed = p.velocity.Normalized();
			float dist_to_col = 0.f;
			if (math::Plane::IntersectLinePlane(geo.plane.normal, geo.plane.d, p.position, norm_speed, dist_to_col))
			{
				p.position += norm_speed * dist_to_col;

				// Resolve impulse only if particle not already moving away from plane
				float dot = p.velocity.Dot(geo.plane.normal);
				if (dot < 0.f)
					p.velocity -= (p.col_restitution + restitution) * dot * geo.plane.normal;
			}
			else // Direction is parallel to plane
				p.position += geo.plane.normal * dist_to_plane;
		}

		break;
	}
	case Type::SPHERE:
	{
		float overlap_distance = p.position.DistanceSq(geo.sphere.pos) - (geo.sphere.r * geo.sphere.r);
		if (overlap_distance > 0.f)
		{
#ifdef PARTICLE_PHYSICS_TEST
			ProfilingTimer::p_col_boundary++;
#endif // PARTICLE_PHYSICS_TEST

			if (effect == RE_EmissionBoundary::KILL) return false;

			// Resolve intersection
			p.position -= p.velocity.Normalized() * math::Sqrt(overlap_distance);

			// Resolve impulse only if particle not already moving away from sphere
			const math::vec impact_normal = (geo.sphere.pos - p.position).Normalized();
			float dot = p.velocity.Dot(impact_normal);
			if (dot < 0.f)
				p.velocity -= (p.col_restitution + restitution) * dot * impact_normal;
		}

		break;
	}
	case Type::AABB:
	{
		int collision = 0;
		for (int i = 5; i >= 0; --i)
		{
			collision = collision << 1;
			const int axis = i % 3;
			collision += (i < 3) ?
				(p.position[axis] <= geo.box.minPoint[axis]) :
				(p.position[axis] >= geo.box.maxPoint[axis]);
		}

		if (collision)
		{
#ifdef PARTICLE_PHYSICS_TEST
			ProfilingTimer::p_col_boundary++;
#endif // PARTICLE_PHYSICS_TEST

			if (effect == RE_EmissionBoundary::KILL) return false;

			for (int i = 0; i < 6; ++i)
			{
				if (collision & (1 << i))
				{
					const int axis = i % 3;
					p.position[axis] = i < 3 ? geo.box.minPoint[axis] : geo.box.maxPoint[axis];

					math::vec normal = math::vec::zero;
					normal[axis] = i < 3 ? 1.f : -1.f;
					float dot = p.velocity.Dot(normal);
					if (dot < 0.f) p.velocity -= (p.col_restitution + restitution) * dot * normal;
				}
			}
		}

		break;
	}
	default: break;
	}

	return true;
}

bool RE_EmissionBoundary::SphereCollision(RE_Particle& p) const
{
	RE_PROFILE(PROF_ParticleBoundSCol, PROF_ParticleBoundary);
	switch (type)
	{
	case Type::PLANE:
	{
		// Check if particle intersects or has passed plane
		float dist_to_plane = geo.plane.SignedDistance(p.position);
		if (dist_to_plane < p.col_radius)
		{
#ifdef PARTICLE_PHYSICS_TEST
			ProfilingTimer::p_col_boundary++;
#endif // PARTICLE_PHYSICS_TEST

			if (effect == RE_EmissionBoundary::KILL) return false;

			// Resolve intersection
			const math::vec norm_speed = p.velocity.Normalized();
			float dist_to_col = 0.f;
			if (math::Plane::IntersectLinePlane(
				geo.plane.normal,
				geo.plane.d + p.col_radius,
				p.position,
				norm_speed,
				dist_to_col))
			{
				p.position += norm_speed * dist_to_col;

				// Resolve impulse only if particle not already moving away from plane
				float dot = p.velocity.Dot(geo.plane.normal);
				if (dot < 0.f)
					p.velocity -= (p.col_restitution + restitution) * dot * geo.plane.normal;
			}
			else // Direction is parallel to plane
				p.position += geo.plane.normal * dist_to_plane;
		}

		break;
	}
	case Type::SPHERE:
	{
		float overlap_distance = p.position.Distance(geo.sphere.pos) + p.col_radius - geo.sphere.r;
		if (overlap_distance > 0.f)
		{
#ifdef PARTICLE_PHYSICS_TEST
			ProfilingTimer::p_col_boundary++;
#endif // PARTICLE_PHYSICS_TEST

			if (effect == RE_EmissionBoundary::KILL) return false;

			// Resolve intersection
			p.position -= p.velocity.Normalized() * overlap_distance;

			// Resolve impulse only if particle not already moving away from sphere
			const math::vec impact_normal = (geo.sphere.pos - p.position).Normalized();
			float dot = p.velocity.Dot(impact_normal);
			if (dot < 0.f)
				p.velocity -= (p.col_restitution + restitution) * dot * impact_normal;
		}
		
		break;
	}
	case Type::AABB:
	{
		int collision = 0;
		for (int i = 5; i >= 0; --i)
		{
			collision = collision << 1;
			const int axis = i % 3;
			collision += (i < 3) ? 
				(p.position[axis] <= geo.box.minPoint[axis] + p.col_radius) :
				(p.position[axis] >= geo.box.maxPoint[axis] - p.col_radius) ;
		}

		if (collision)
		{
#ifdef PARTICLE_PHYSICS_TEST
			ProfilingTimer::p_col_boundary++;
#endif // PARTICLE_PHYSICS_TEST

			if (effect == RE_EmissionBoundary::KILL) return false;

			for (int i = 0; i < 6; ++i)
			{
				if (collision & (1 << i))
				{
					const int axis = i % 3;
					p.position[axis] = i < 3 ? geo.box.minPoint[axis] + p.col_radius : geo.box.maxPoint[axis] - p.col_radius;

					math::vec normal = math::vec::zero;
					normal[axis] = i < 3 ? 1.f : -1.f;
					float dot = p.velocity.Dot(normal);
					if (dot < 0.f) p.velocity -= (p.col_restitution + restitution) * dot * normal;
				}
			}
		}
		break;
	}
	default: break;
	}

	return true;
}

bool RE_EmissionBoundary::DrawEditor()
{
	bool ret = false;
	int tmp = static_cast<int>(type);
	if (ImGui::Combo("Boundary Type", &tmp, "None\0Plane\0Sphere\0AABB\0"))
	{
		switch (type = static_cast<Type>(tmp)) {
		case RE_EmissionBoundary::PLANE: geo.plane = math::Plane({ 0.f, 1.f, 0.f }, 0.f); break;
		case RE_EmissionBoundary::SPHERE: geo.sphere = math::Sphere({ 0.f, 0.f, 0.f }, 10.f); break;
		case RE_EmissionBoundary::AABB: geo.box.SetFromCenterAndSize(math::vec::zero, math::vec::one * 5.f); break; 
		default: break; }
		ret = true;
	}

	if (type)
	{
		tmp = static_cast<int>(effect);
		if (ImGui::Combo("Boundary Effect", &tmp, "Contain\0Kill\0")) {
			effect = static_cast<Effect>(tmp);
			ret = true;
		}

		if (effect == Effect::CONTAIN)
			if (ImGui::DragFloat("Boundary Restitution", &restitution, 1.f, 0.f, 100.f)) ret = true;

		switch (type) {
		case RE_EmissionBoundary::NONE: break;
		case RE_EmissionBoundary::PLANE:
		{
			if (ImGui::DragFloat("Distance to origin", &geo.plane.d, 1.f, 0.f)) ret = true;
			math::float2 angles = geo.plane.normal.ToSphericalCoordinatesNormalized() * RE_Math::rad_to_deg;
			if (ImGui::DragFloat2("Boundary Yaw - Pitch", angles.ptr(), 0.1f, -180.f, 180.f))
			{
				angles *= RE_Math::deg_to_rad;
				geo.plane.normal = math::vec::FromSphericalCoordinates(angles.x, angles.y);
				ret = true;
			}

			break;
		}
		case RE_EmissionBoundary::SPHERE:
		{
			if (ImGui::DragFloat3("Boundary Position", geo.sphere.pos.ptr())) ret = true;
			if (ImGui::DragFloat("Boundary Radius", &geo.sphere.r, 1.f, 0.f)) ret = true;
			break;
		}
		case RE_EmissionBoundary::AABB:
		{
			if(ImGui::DragFloat3("Boundary Min", geo.box.minPoint.ptr())) ret = true;
			if(ImGui::DragFloat3("Boundary Max", geo.box.maxPoint.ptr())) ret = true;
			break;
		}
		}
	}
	return ret;
}

void RE_EmissionBoundary::JsonDeserialize(RE_Json* node)
{
	type = static_cast<RE_EmissionBoundary::Type>(node->PullInt("Type", static_cast<int>(type)));
	if (type)
	{
		effect = static_cast<RE_EmissionBoundary::Effect>(node->PullInt("Effect", static_cast<int>(effect)));

		switch (type) {
		case RE_EmissionBoundary::PLANE:
		{
			geo.plane.d = node->PullFloat("Distance", geo.plane.d);
			geo.plane.normal = node->PullFloatVector("Normal", geo.plane.normal);
			break;
		}
		case RE_EmissionBoundary::SPHERE:
		{
			geo.sphere.r = node->PullFloat("Radius", geo.sphere.r);
			geo.sphere.pos = node->PullFloatVector("Position", geo.sphere.pos);
			break;
		}
		case RE_EmissionBoundary::AABB:
		{
			geo.box.minPoint = node->PullFloatVector("Min point", geo.box.minPoint);
			geo.box.maxPoint = node->PullFloatVector("Max point", geo.box.maxPoint);
			break;
		}
		default: break; }
	}

	DEL(node);
}

void RE_EmissionBoundary::JsonSerialize(RE_Json* node) const
{
	node->PushInt("Type", static_cast<int>(type));
	if (type)
	{
		node->PushInt("Effect", static_cast<int>(effect));

		switch (type) {
		case RE_EmissionBoundary::PLANE:
		{
			node->PushFloat("Distance", geo.plane.d);
			node->PushFloatVector("Normal", geo.plane.normal);
			break;
		}
		case RE_EmissionBoundary::SPHERE:
		{
			node->PushFloat("Radius", geo.sphere.r);
			node->PushFloatVector("Position", geo.sphere.pos);
			break;
		}
		case RE_EmissionBoundary::AABB:
		{
			node->PushFloatVector("Min point", geo.box.minPoint);
			node->PushFloatVector("Max point", geo.box.maxPoint);
			break;
		}
		default: break; }
	}

	DEL(node);
}

void RE_EmissionBoundary::BinaryDeserialize(char*& cursor)
{
	unsigned int size = sizeof(int);
	memcpy(&type, cursor, size);
	cursor += size;
	if (type)
	{
		memcpy(&effect, cursor, size);
		cursor += size;

		switch (type) {
		case RE_EmissionBoundary::PLANE:
		{
			size = sizeof(float);
			memcpy(&geo.plane.d, cursor, size);
			cursor += size;
			size *= 3u;
			memcpy(geo.plane.normal.ptr(), cursor, size);
			cursor += size;
			break;
		}
		case RE_EmissionBoundary::SPHERE:
		{
			size = sizeof(float);
			memcpy(&geo.sphere.r, cursor, size);
			cursor += size;
			size *= 3u;
			memcpy(geo.sphere.pos.ptr(), cursor, size);
			cursor += size;
			break;
		}
		case RE_EmissionBoundary::AABB:
		{
			size = sizeof(float) * 3u;
			memcpy(geo.box.minPoint.ptr(), cursor, size);
			cursor += size;
			memcpy(geo.box.maxPoint.ptr(), cursor, size);
			cursor += size;
			break;
		}
		default: break;
		}
	}
}

void RE_EmissionBoundary::BinarySerialize(char*& cursor) const
{
	unsigned int size = sizeof(int);
	memcpy(cursor, &type, size);
	cursor += size;
	if (type)
	{
		memcpy(cursor, &effect, size);
		cursor += size;

		switch (type) {
		case RE_EmissionBoundary::PLANE:
		{
			size = sizeof(float);
			memcpy(cursor, &geo.plane.d, size);
			cursor += size;
			size *= 3u;
			memcpy(cursor, geo.plane.normal.ptr(), size);
			cursor += size;
			break;
		}
		case RE_EmissionBoundary::SPHERE:
		{
			size = sizeof(float);
			memcpy(cursor, &geo.sphere.r, size);
			cursor += size;
			size *= 3u;
			memcpy(cursor, geo.sphere.pos.ptr(), size);
			cursor += size;
			break;
		}
		case RE_EmissionBoundary::AABB:
		{
			size = sizeof(float) * 3u;
			memcpy(cursor, geo.box.minPoint.ptr(), size);
			cursor += size;
			memcpy(cursor, geo.box.maxPoint.ptr(), size);
			cursor += size;
			break;
		}
		default: break;
		}
	}
}

unsigned int RE_EmissionBoundary::GetBinarySize() const
{
	unsigned int ret = sizeof(int);
	if (type)
	{
		ret *= 2u;
		switch (type) {
		case RE_EmissionBoundary::Type::PLANE: ret += sizeof(float) * 4u; break;
		case RE_EmissionBoundary::Type::SPHERE: ret += sizeof(float) * 4u; break;
		case RE_EmissionBoundary::Type::AABB: ret += sizeof(float) * 6u; break;
		default: break; }
	}

	return ret;
}

bool RE_EmissionCollider::DrawEditor()
{
	bool ret = false;
	int tmp = static_cast<int>(type);
	if (ImGui::Combo("Collider Type", &tmp, "None\0Point\0Sphere\0")) {
		type = static_cast<Type>(tmp);
		ret = true;
	}

	if (type)
	{
		if(ImGui::Checkbox("Inter collisions", &inter_collisions)) ret = true;

		if (mass.DrawEditor("Mass")) ret = true;
		if (restitution.DrawEditor("Restitution")) ret = true;

		if (type == RE_EmissionCollider::SPHERE)
			if (radius.DrawEditor("Collider Radius")) ret = true;
	}
	return ret;
}

void RE_EmissionCollider::JsonDeserialize(RE_Json* node)
{
	type = static_cast<RE_EmissionCollider::Type>(node->PullInt("Type", static_cast<int>(type)));
	if (type)
	{
		inter_collisions = node->PullBool("Inter collisions", inter_collisions);
		switch (type) {
		case RE_EmissionCollider::Type::POINT:
		{
			mass.JsonDeserialize(node->PullJObject("Mass"));
			restitution.JsonDeserialize(node->PullJObject("Restitution"));
			break;
		}
		case RE_EmissionCollider::Type::SPHERE:
		{
			mass.JsonDeserialize(node->PullJObject("Mass"));
			radius.JsonDeserialize(node->PullJObject("Radius"));
			restitution.JsonDeserialize(node->PullJObject("Restitution"));
			break;
		}
		default: break; }
	}

	DEL(node);
}

void RE_EmissionCollider::JsonSerialize(RE_Json* node) const
{
	node->PushInt("Type", static_cast<int>(type));
	if (type)
	{
		node->PushBool("Inter collisions", inter_collisions);
		switch (type) {
		case RE_EmissionCollider::Type::POINT:
		{
			mass.JsonSerialize(node->PushJObject("Mass"));
			restitution.JsonSerialize(node->PushJObject("Restitution"));
			break;
		}
		case RE_EmissionCollider::Type::SPHERE:
		{
			mass.JsonSerialize(node->PushJObject("Mass"));
			radius.JsonSerialize(node->PushJObject("Radius"));
			restitution.JsonSerialize(node->PushJObject("Restitution"));
			break;
		}
		default: break; }
	}

	DEL(node);
}

void RE_EmissionCollider::BinaryDeserialize(char*& cursor)
{
	unsigned size = sizeof(int);
	memcpy(&type, cursor, size);
	cursor += size;

	if (type)
	{
		size = sizeof(bool);
		memcpy(&inter_collisions, cursor, size);
		cursor += size;

		switch (type) {
		case RE_EmissionCollider::Type::POINT:
		{
			mass.BinaryDeserialize(cursor);
			restitution.BinaryDeserialize(cursor);
			break;
		}
		case RE_EmissionCollider::Type::SPHERE:
		{
			mass.BinaryDeserialize(cursor);
			radius.BinaryDeserialize(cursor);
			restitution.BinaryDeserialize(cursor);
			break;
		}
		default: break;
		}
	}
}

void RE_EmissionCollider::BinarySerialize(char*& cursor) const
{
	unsigned size = sizeof(int);
	memcpy(cursor, &type, size);
	cursor += size;

	if (type)
	{
		size = sizeof(bool);
		memcpy(cursor, &inter_collisions, size);
		cursor += size;

		switch (type) {
		case RE_EmissionCollider::Type::POINT:
		{
			mass.BinarySerialize(cursor);
			restitution.BinarySerialize(cursor);
			break;
		}
		case RE_EmissionCollider::Type::SPHERE:
		{
			mass.BinarySerialize(cursor);
			radius.BinarySerialize(cursor);
			restitution.BinarySerialize(cursor);
			break;
		}
		default: break; }
	}
}

unsigned int RE_EmissionCollider::GetBinarySize() const
{
	unsigned int ret = sizeof(int);
	if (type)
	{
		ret += sizeof(bool);
		switch (type) {
		case RE_EmissionCollider::Type::POINT:
		{
			ret += mass.GetBinarySize();
			ret += restitution.GetBinarySize();
			break;
		}
		case RE_EmissionCollider::Type::SPHERE:
		{
			ret += mass.GetBinarySize();
			ret += radius.GetBinarySize();
			ret += restitution.GetBinarySize();
			break;
		}
		default: break;
		}
	}

	return ret;
}

math::vec RE_PR_Color::GetValue(const float weight) const
{
	switch (type)
	{
	case RE_PR_Color::SINGLE: return base;
	default:
	{
		 float w = (useCurve) ? curve.GetValue(weight) : weight;
		 return (gradient * w) + (base * (1.f - w));
	}
	}
}

bool RE_PR_Color::DrawEditor()
{
	bool ret = false;
	int tmp = static_cast<int>(type);
	if (ImGui::Combo("Color Type", &tmp, "Single\0Over Lifetime\0Over Distance\0Over Speed\0")) {
		type = static_cast<Type>(tmp);
		ret = true;
	}

	if (type != RE_PR_Color::SINGLE)
	{
		if (ImGui::ColorEdit3("Particle Gradient 1", base.ptr())) ret = true;
		if (ImGui::ColorEdit3("Particle Gradient 2", gradient.ptr())) ret = true;

		if (ImGui::Checkbox(useCurve ? "Disable Color Curve" : "Enable Color Curve", &useCurve)) ret = true;
	}
	else
		if (ImGui::ColorEdit3("Particle Color", base.ptr())) ret = true;

	return ret;
}

void RE_PR_Color::JsonDeserialize(RE_Json* node)
{
	type = static_cast<Type>(node->PullInt("Type", 0));

	base = node->PullFloatVector("Base", math::vec::one);
	gradient = node->PullFloatVector("Gradient", math::vec::zero);
	useCurve = node->PullBool("useCurve", false);

	curve.JsonDeserialize(node->PullJObject("curve"));

	DEL(node);
}

void RE_PR_Color::JsonSerialize(RE_Json* node) const
{
	node->PushInt("Type", static_cast<int>(type));

	node->PushFloatVector("Base", base);
	node->PushFloatVector("Gradient", gradient);
	node->PushBool("useCurve", useCurve);

	curve.JsonSerialize(node->PushJObject("curve"));

	DEL(node);
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

unsigned int RE_PR_Color::GetBinarySize() const
{
	return sizeof(int)
		+ (sizeof(float) * 6u)
		+ sizeof(bool)
		+ curve.GetBinarySize();
}

CurveData::CurveData()
{
	points.push_back({ -1.0f, 0.0f });
	for (int i = 1; i < total_points; i++)
		points.push_back({ 0.0f, 0.0f });
}

CurveData::~CurveData() { points.clear(); }

float CurveData::GetValue(const float weight) const
{
	return smooth ?
		ImGui::CurveValueSmooth(weight, total_points, points.data()) :
		ImGui::CurveValue(weight, total_points, points.data());
}

bool CurveData::DrawEditor(const char* name)
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

void CurveData::JsonDeserialize(RE_Json* node)
{
	smooth = node->PullBool("Smooth", false);
	total_points = node->PullInt("TotalPoints", 10);
	comboCurve = node->PullInt("comboCurve", 0);
	for (int i = 0; i < total_points; i++) {
		math::float2 toImVec2 = node->PullFloat((eastl::to_string(i) + "p").c_str(), { -1.0f, 0.0f });
		points.push_back({ toImVec2.x,toImVec2.y });
	}
	DEL(node);
}

void CurveData::JsonSerialize(RE_Json* node) const
{
	node->PushBool("Smooth", smooth);
	node->PushInt("TotalPoints", total_points);
	node->PushInt("comboCurve", comboCurve);
	for (int i = 0; i < total_points; i++)
		node->PushFloat((eastl::to_string(i) + "p").c_str(), { points[i].x, points[i].y });

	DEL(node);
}

void CurveData::BinaryDeserialize(char*& cursor)
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

void CurveData::BinarySerialize(char*& cursor) const
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

unsigned int CurveData::GetBinarySize() const
{
	return sizeof(bool) + (sizeof(int) * 2u) + (sizeof(float) * 2u * points.size());
}

float RE_PR_Opacity::GetValue(const float weight) const
{
	switch (type) {
	case RE_PR_Opacity::VALUE: return opacity;
	case RE_PR_Opacity::OVERLIFETIME:
	case RE_PR_Opacity::OVERDISTANCE:
	case RE_PR_Opacity::OVERSPEED:
		return (useCurve) ? curve.GetValue(weight) : (inverted) ? 1.f - weight : weight;
	default: return 1.f; }
}

bool RE_PR_Opacity::DrawEditor()
{
	bool ret = false;
	int tmp = static_cast<int>(type);
	if (ImGui::Combo("Opacity Type", &tmp, "None\0Value\0Over Lifetime\0Over Distance\0Over Speed\0")) {
		type = static_cast<Type>(tmp);
		ret = true;
	}

	switch (type)
	{
	case RE_PR_Opacity::VALUE:
		if (ImGui::SliderFloat("Opacity", &opacity, 0.0f, 1.0f)) ret = true;
		break;
	case RE_PR_Opacity::OVERLIFETIME:
	case RE_PR_Opacity::OVERDISTANCE:
	case RE_PR_Opacity::OVERSPEED:
		if (ImGui::Checkbox(useCurve ? "Disable Opacity Curve" : "Enable Opacity Curve", &useCurve)) ret = true;
		if (!useCurve) {
			ImGui::SameLine();
			if (ImGui::Checkbox("Invert Opacity", &inverted)) ret = true;
		}
		break;
	default:
		break;
	}
	return ret;
}

void RE_PR_Opacity::JsonDeserialize(RE_Json* node)
{
	type = static_cast<Type>(node->PullInt("Type", 0));

	opacity = node->PullFloat("Opacity", 1.0f);
	inverted = node->PullBool("Inverted", false);
	useCurve = node->PullBool("useCurve", false);

	curve.JsonDeserialize(node->PullJObject("curve"));
	DEL(node);
}

void RE_PR_Opacity::JsonSerialize(RE_Json* node) const
{
	node->PushInt("Type", static_cast<int>(type));

	node->PushFloat("Opacity", opacity);
	node->PushBool("Inverted", inverted);
	node->PushBool("useCurve", useCurve);

	curve.JsonSerialize(node->PushJObject("curve"));

	DEL(node);
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

unsigned int RE_PR_Opacity::GetBinarySize() const
{
	return sizeof(int)
		+ sizeof(float)
		+ (sizeof(bool) * 2)
		+ curve.GetBinarySize();
}

math::vec RE_PR_Light::GetColor() const { return random_color ? RE_MATH->RandomVec() : color; }
float RE_PR_Light::GetIntensity() const { return random_i ? intensity + (RE_MATH->RandomF() * (intensity_max - intensity)) : intensity; }
float RE_PR_Light::GetSpecular() const { return random_s ? specular + (RE_MATH->RandomF() * (specular_max - specular)) : specular; }
math::vec RE_PR_Light::GetQuadraticValues() const { return { constant, linear, quadratic }; }

bool RE_PR_Light::DrawEditor(const unsigned int id)
{
	bool ret = false;
	int tmp = static_cast<int>(type);
	if (ImGui::Combo("Light Mode", &tmp, "None\0Unique\0Per Particle\0"))
	{
		switch (type = static_cast<Type>(tmp)) {
		case RE_PR_Light::UNIQUE:
		{
			auto particles = RE_PHYSICS->GetParticles(id);
			color = GetColor();
			intensity = GetIntensity();
			specular = GetSpecular();

			for (auto &p : particles)
			{
				p.lightColor = color;
				p.intensity = intensity;
				p.specular = specular;
			}

			break;
		}
		case RE_PR_Light::PER_PARTICLE:
		{
			auto particles = RE_PHYSICS->GetParticles(id);
			for (auto &p : particles)
			{
				p.lightColor = GetColor();
				p.intensity = GetIntensity();
				p.specular = GetSpecular();
			}

			break;
		}
		default: break; }

		ret = true;
	}

	switch (type) {
	case RE_PR_Light::UNIQUE:
	{
		auto particles = RE_PHYSICS->GetParticles(id);

		if (ImGui::ColorEdit3("Light Color", color.ptr())) ret = true;
		if(ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 50.0f, "%.2f")) ret = true;
		if(ImGui::DragFloat("Specular", &specular, 0.01f, 0.f, 1.f, "%.2f")) ret = true;

		ImGui::Separator();
		if(ImGui::DragFloat("Constant", &constant, 0.01f, 0.001f, 5.0f, "%.2f")) ret = true;
		if(ImGui::DragFloat("Linear", &linear, 0.001f, 0.001f, 5.0f, "%.3f")) ret = true;
		if(ImGui::DragFloat("Quadratic", &quadratic, 0.001f, 0.001f, 5.0f, "%.3f")) ret = true;

		break;
	}
	case RE_PR_Light::PER_PARTICLE:
	{
		auto particles = RE_PHYSICS->GetParticles(id);

		if (ImGui::Checkbox("Random Color", &random_color)) {
			for (auto &p : particles) p.lightColor = GetColor();
			ret = true;
		}

		if (!random_color && ImGui::ColorEdit3("Light Color", color.ptr())) {
			for (auto &p : particles) p.lightColor = color;
			ret = true;
		}

		if (ImGui::Checkbox("Random Intensity", &random_i)) {
			for (auto &p : particles) p.intensity = GetIntensity();
			ret = true;
		}

		if (random_i)
		{
			bool update_sim = false;
			update_sim |= ImGui::DragFloat("Intensity Min", &intensity, 0.01f, 0.0f, intensity_max, "%.2f");
			update_sim |= ImGui::DragFloat("Intensity Max", &intensity_max, 0.01f, intensity, 50.f, "%.2f");
			if (update_sim) {
				for (auto &p : particles) p.intensity = GetIntensity();
				ret = true;
			}
		}
		else if (ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 50.0f, "%.2f")) {
			for (auto &p : particles) p.intensity = intensity;
			ret = true;
		}

		if (ImGui::Checkbox("Random Specular", &random_s)) {
			for (auto &p : particles) p.specular = GetSpecular();
			ret = true;
		}

		if (random_s)
		{
			bool update_sim = false;
			update_sim |= ImGui::DragFloat("Specular Min", &specular, 0.01f, 0.0f, specular_max, "%.2f");
			update_sim |= ImGui::DragFloat("Specular Max", &specular_max, 0.01f, specular, 10.f, "%.2f");
			if (update_sim) {
				for (auto &p : particles) p.specular = GetSpecular();
				ret = true;
			}
		}
		else if (ImGui::DragFloat("Specular", &specular, 0.01f, 0.f, 1.f, "%.2f")) {
			for (auto &p : particles) p.specular = specular;
			ret = true;
		}

		ImGui::Separator();
		if(ImGui::DragFloat("Constant", &constant, 0.01f, 0.001f, 5.0f, "%.2f")) ret = true;
		if(ImGui::DragFloat("Linear", &linear, 0.001f, 0.001f, 5.0f, "%.3f")) ret = true;
		if(ImGui::DragFloat("Quadratic", &quadratic, 0.001f, 0.001f, 5.0f, "%.3f")) ret = true;

		break;
	}
	default: break; }

	return ret;
}

void RE_PR_Light::JsonDeserialize(RE_Json* node)
{
	type = static_cast<RE_PR_Light::Type>(node->PullInt("Type", static_cast<int>(type)));

	switch (type) {
	case RE_PR_Light::UNIQUE:
	{
		color = node->PullFloatVector("Color", color);
		intensity = node->PullFloat("Intensity", intensity);
		specular = node->PullFloat("Specular", specular);

		constant = node->PullFloat("Constant", constant);
		linear = node->PullFloat("Linear", linear);
		quadratic = node->PullFloat("Quadratic", quadratic);

		break;
	}
	case RE_PR_Light::PER_PARTICLE:
	{
		random_color = node->PullBool("Random Color", random_color);
		if (!random_color) color = node->PullFloatVector("Color", color);

		random_i = node->PullBool("Random Intensity", random_i);
		intensity = node->PullFloat("Intensity", intensity);
		if (random_i) intensity_max = node->PullFloat("Intensity Max", intensity_max);

		random_s = node->PullBool("Random Specular", random_s);
		specular = node->PullFloat("Specular", specular);
		if (random_s) specular_max = node->PullFloat("Specular Max", specular_max);

		constant = node->PullFloat("Constant", constant);
		linear = node->PullFloat("Linear", linear);
		quadratic = node->PullFloat("Quadratic", quadratic);

		break;
	}
	default: break; }

	DEL(node);
}

void RE_PR_Light::JsonSerialize(RE_Json* node) const
{
	node->PushInt("Type", static_cast<int>(type));
	switch (type) {
	case RE_PR_Light::UNIQUE:
	{
		node->PushFloatVector("Color", color);
		node->PushFloat("Intensity", intensity);
		node->PushFloat("Specular", specular);

		node->PushFloat("Constant", constant);
		node->PushFloat("Linear", linear);
		node->PushFloat("Quadratic", quadratic);

		break;
	}
	case RE_PR_Light::PER_PARTICLE:
	{
		node->PushBool("Random Color", random_color);
		if (!random_color) node->PushFloatVector("Color", color);

		node->PushBool("Random Intensity", random_i);
		node->PushFloat("Intensity", intensity);
		if (random_i) node->PushFloat("Intensity Max", intensity_max);

		node->PushBool("Random Specular", random_s);
		node->PushFloat("Specular", specular);
		if (random_s) node->PushFloat("Specular Max", specular_max);

		node->PushFloat("Constant", constant);
		node->PushFloat("Linear", linear);
		node->PushFloat("Quadratic", quadratic);

		break;
	}
	default: break; }

	DEL(node);
}

void RE_PR_Light::BinaryDeserialize(char*& cursor)
{
	unsigned int size = sizeof(int);
	memcpy(&type, cursor, size);
	cursor += size;

	switch (type) {
	case RE_PR_Light::UNIQUE:
	{
		size = sizeof(float);
		memcpy(color.ptr(), cursor, size * 3u);
		cursor += size * 3u;
		memcpy(&intensity, cursor, size);
		cursor += size;
		memcpy(&specular, cursor, size);
		cursor += size;

		memcpy(&constant, cursor, size);
		cursor += size;
		memcpy(&linear, cursor, size);
		cursor += size;
		memcpy(&quadratic, cursor, size);
		cursor += size;

		break;
	}
	case RE_PR_Light::PER_PARTICLE:
	{
		size = sizeof(bool);
		memcpy(&random_color, cursor, size);
		cursor += size;
		if (!random_color)
		{
			size = sizeof(float) * 3u;
			memcpy(color.ptr(), cursor, size);
			cursor += size;
		}

		size = sizeof(bool);
		memcpy(&random_i, cursor, size);
		cursor += size;
		size = sizeof(float);
		memcpy(&intensity, cursor, size);
		if (!random_i)
		{
			memcpy(&intensity_max, cursor, size);
			cursor += size;
		}

		size = sizeof(bool);
		memcpy(&random_s, cursor, size);
		cursor += size;
		size = sizeof(float);
		memcpy(&specular, cursor, size);
		if (!random_s)
		{
			memcpy(&specular_max, cursor, size);
			cursor += size;
		}

		memcpy(&constant, cursor, size);
		cursor += size;
		memcpy(&linear, cursor, size);
		cursor += size;
		memcpy(&quadratic, cursor, size);
		cursor += size;

		break;
	}
	default: break; }
}

void RE_PR_Light::BinarySerialize(char*& cursor) const
{
	unsigned int size = sizeof(int);
	memcpy(cursor, &type, size);
	cursor += size;

	switch (type) {
	case RE_PR_Light::UNIQUE:
	{
		size = sizeof(float);
		memcpy(cursor, color.ptr(), size * 3u);
		cursor += size * 3u;
		memcpy(cursor, &intensity, size);
		cursor += size;
		memcpy(cursor, &specular, size);
		cursor += size;

		memcpy(cursor, &constant, size);
		cursor += size;
		memcpy(cursor, &linear, size);
		cursor += size;
		memcpy(cursor, &quadratic, size);
		cursor += size;

		break;
	}
	case RE_PR_Light::PER_PARTICLE:
	{
		size = sizeof(bool);
		memcpy(cursor, &random_color, size);
		cursor += size;
		if (!random_color)
		{
			size = sizeof(float) * 3u;
			memcpy(cursor, color.ptr(), size);
			cursor += size;
		}

		size = sizeof(bool);
		memcpy(cursor, &random_i, size);
		cursor += size;
		size = sizeof(float);
		memcpy(cursor, &intensity, size);
		if (!random_i)
		{
			memcpy(cursor, &intensity_max, size);
			cursor += size;
		}

		size = sizeof(bool);
		memcpy(cursor, &random_s, size);
		cursor += size;
		size = sizeof(float);
		memcpy(cursor, &specular, size);
		if (!random_s)
		{
			memcpy(cursor, &specular_max, size);
			cursor += size;
		}

		memcpy(cursor, &constant, size);
		cursor += size;
		memcpy(cursor, &linear, size);
		cursor += size;
		memcpy(cursor, &quadratic, size);
		cursor += size;

		break;
	}
	default: break;
	}
}

unsigned int RE_PR_Light::GetBinarySize() const
{
	unsigned int ret = sizeof(int);

	switch (type) {
	case RE_PR_Light::UNIQUE:
	{
		ret += sizeof(float) * 8u;
		break;
	}
	case RE_PR_Light::PER_PARTICLE:
	{
		ret += sizeof(bool) * 3u;
		ret += sizeof(float) * 5u;

		if (!random_color) ret += sizeof(float) * 3u;
		if (!random_i) ret += sizeof(float);
		if (!random_s) ret += sizeof(float);

		break;
	}
	default: break; }

	return ret;
}
