#include "RE_EmissionData.h"

#include "Application.h"
#include "RE_Math.h"
#include "RE_Particle.h"
#include "RE_Json.h"
#include "ImGui\imgui.h"

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

bool RE_EmissionInterval::DrawEditor()
{
	bool ret = false;
	int tmp = static_cast<int>(type);
	if (ImGui::Combo("Interval", &tmp, "None\0Intermitent\0Custom\0"))
	{
		type = static_cast<RE_EmissionInterval::Type>(tmp);
		is_open = true;
		time_offset = 0.f;
		duration[0] = duration[1] = 1.f;
		ret = true;
	}

	switch (type) {
	case RE_EmissionInterval::INTERMITENT:
	{
		ImGui::DragFloat("Interval On", &duration[1], 1.f, 0.f, 10000.f);
		break; 
	}
	case RE_EmissionInterval::CUSTOM:
	{
		ImGui::DragFloat("Interval On", &duration[1], 1.f, 0.f, 10000.f);
		ImGui::DragFloat("Interval Off", &duration[0], 1.f, 0.f, 10000.f);
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

bool RE_EmissionSpawn::DrawEditor()
{
	bool ret = false;
	int tmp = static_cast<int>(type);
	if (ImGui::Combo("Spawn type", &tmp, "Single\0Burst\0Flow\0"))
	{
		type = static_cast<RE_EmissionSpawn::Type>(tmp);
		has_started = false;
		time_offset = 0.f;
		ret = true;
	}

	switch (type) {
	case RE_EmissionSpawn::Type::SINGLE:
	{
		ImGui::DragInt("Particle amount", &particles_spawned, 1.f, 0, 10000);
		break;
	}
	case RE_EmissionSpawn::Type::BURST:
	{
		ImGui::DragInt("Particles/burst", &particles_spawned, 1.f, 0, 10000);
		ImGui::DragFloat("Period", &frequency, 1.f, 0.0001f, 10000.f);

		break;
	}
	case RE_EmissionSpawn::Type::FLOW:
	{
		ImGui::DragFloat("Frecuency", &frequency, 1.f, 0.0001f, 1000.f);
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
	switch (shape) {
	case CIRCLE: return geo.circle.GetPoint(RE_MATH->RandomF() * RE_Math::pi_x2);
	case RING: return geo.ring.first.GetPoint(RE_MATH->RandomF() * RE_Math::pi_x2) + (RE_MATH->RandomNDir() * geo.ring.second);
	case AABB: return { 
			geo.box.minPoint.x + (RE_MATH->RandomF() * (geo.box.maxPoint.x - geo.box.minPoint.x)),
			geo.box.minPoint.y + (RE_MATH->RandomF() * (geo.box.maxPoint.y - geo.box.minPoint.y)),
			geo.box.minPoint.z + (RE_MATH->RandomF() * (geo.box.maxPoint.z - geo.box.minPoint.z)) };
	case SPHERE: return geo.sphere.pos + ((RE_MATH->RandomF() * geo.sphere.r) * RE_MATH->RandomNDir());
	case HOLLOW_SPHERE: return geo.hollow_sphere.first.pos + ((geo.hollow_sphere.first.r + (geo.hollow_sphere.second + RE_MATH->RandomFN())) * RE_MATH->RandomNDir());
	default: return geo.point; }
}

void RE_EmissionShape::DrawEditor()
{
	int next_shape = static_cast<int>(shape);
	if (ImGui::Combo("Emissor Shape", &next_shape, "Point\0Cirle\0Ring\0AABB\0Sphere\0Hollow Sphere\0"))
	{
		switch (shape = static_cast<Type>(next_shape)) {
		case RE_EmissionShape::POINT: geo.point = math::vec::zero; break;
		case RE_EmissionShape::CIRCLE: geo.circle = math::Circle(math::vec::zero, { 0.f, 1.f, 0.f }, 1.f); break;
		case RE_EmissionShape::RING: geo.ring = { math::Circle(math::vec::zero, { 0.f, 1.f, 0.f }, 1.f), 0.1f }; break;
		case RE_EmissionShape::AABB: geo.box.SetFromCenterAndSize(math::vec::zero, math::vec::one); break;
		case RE_EmissionShape::SPHERE: geo.sphere = math::Sphere(math::vec::zero, 1.f); break;
		case RE_EmissionShape::HOLLOW_SPHERE: geo.hollow_sphere = { math::Sphere(math::vec::zero, 1.f), 0.8f }; break; }
	}

	switch (shape)
	{
	case RE_EmissionShape::POINT:
	{
		ImGui::DragFloat3("Starting Pos", geo.point.ptr());
		break;
	}
	case RE_EmissionShape::CIRCLE:
	{
		ImGui::DragFloat3("Circle Origin", geo.circle.pos.ptr());
		ImGui::DragFloat("Circle Radius", &geo.circle.r, 1.f, 0.f);

		math::float2 angles = geo.circle.normal.ToSphericalCoordinatesNormalized() * RE_Math::rad_to_deg;
		if (ImGui::DragFloat2("Circle Yaw - Pitch", angles.ptr(), 0.1f, -180.f, 180.f))
		{
			angles *= RE_Math::deg_to_rad;
			geo.circle.normal = math::vec::FromSphericalCoordinates(angles.x, angles.y);
		}
		break;
	}
	case RE_EmissionShape::RING:
	{
		ImGui::DragFloat3("Ring Origin", geo.ring.first.pos.ptr());
		ImGui::DragFloat("Ring Radius", &geo.ring.first.r, 1.f, 0.f);
		ImGui::DragFloat("Ring Inner Radius", &geo.ring.second, 1.f, 0.f, geo.ring.first.r);

		math::float2 angles = geo.ring.first.normal.ToSphericalCoordinatesNormalized() * RE_Math::rad_to_deg;
		if (ImGui::DragFloat2("Ring Yaw - Pitch", angles.ptr(), 0.1f, -180.f, 180.f))
		{
			angles *= RE_Math::deg_to_rad;
			geo.ring.first.normal = math::vec::FromSphericalCoordinates(angles.x, angles.y);
		}
		break;
	}
	case RE_EmissionShape::AABB:
	{
		ImGui::DragFloat3("Box Min ", geo.box.minPoint.ptr());
		ImGui::DragFloat3("Box Max ", geo.box.maxPoint.ptr());
		break;
	}
	case RE_EmissionShape::SPHERE:
	{
		ImGui::DragFloat3("Sphere Origin", geo.sphere.pos.ptr());
		ImGui::DragFloat("Sphere Radius", &geo.sphere.r, 1.f, 0.f);
		break;
	}
	case RE_EmissionShape::HOLLOW_SPHERE:
	{
		ImGui::DragFloat3("Hollow Sphere Origin", geo.hollow_sphere.first.pos.ptr());
		ImGui::DragFloat("Hollow Sphere Radius", &geo.hollow_sphere.first.r, 1.f, 0.f);
		ImGui::DragFloat("Hollow Sphere Inner Radius", &geo.hollow_sphere.second, 1.f, 0.f, geo.hollow_sphere.first.r);
		break;
	}
	}
}

void RE_EmissionShape::JsonDeserialize(RE_Json* node)
{
	shape = static_cast<RE_EmissionShape::Type>(node->PullInt("Type", static_cast<int>(shape)));
	switch (shape) {
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
	node->PushInt("Type", static_cast<int>(shape));
	switch (shape) {
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
	memcpy(&shape, cursor, size);
	cursor += size;
	switch (shape) {
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
	memcpy(cursor, &shape, size);
	cursor += size;
	switch (shape) {
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
	switch (shape) {
	case RE_EmissionShape::Type::CIRCLE: ret += sizeof(float) * 7u; break;
	case RE_EmissionShape::Type::RING: ret += sizeof(float) * 8u; break;
	case RE_EmissionShape::Type::AABB: ret += sizeof(float) * 6u; break;
	case RE_EmissionShape::Type::SPHERE: ret += sizeof(float) * 4u; break;
	case RE_EmissionShape::Type::HOLLOW_SPHERE: ret += sizeof(float) * 5u; break;
	default: ret += sizeof(float) * 3u; break; }
	return ret;
}

math::vec RE_EmissionVector::GetSpeed() const
{
	switch (type) {
	case VALUE: return val;
	case RANGEX: return { val.x + (RE_MATH->RandomFN() * margin.x), val.y, val.z};
	case RANGEY: return { val.x, val.y + (RE_MATH->RandomFN() * margin.y), val.z };
	case RANGEZ: return { val.x, val.y, val.z + (RE_MATH->RandomFN() * margin.z) };
	case RANGEXY: return { val.x + (RE_MATH->RandomFN() * margin.x), val.y + (RE_MATH->RandomFN() * margin.y), val.z };
	case RANGEXZ: return { val.x + (RE_MATH->RandomFN() * margin.x), val.y, val.z + (RE_MATH->RandomFN() * margin.z) };
	case RANGEYZ: return { val.x, val.y + (RE_MATH->RandomFN() * margin.y), val.z + (RE_MATH->RandomFN() * margin.z) };
	case RANGEXYZ: return val.Mul(margin);
	default: return math::vec::zero; }
}

void RE_EmissionVector::DrawEditor(const char* name)
{
	const eastl::string tmp(name);
	int next_type = static_cast<int>(type);
	if (ImGui::Combo((tmp + " type").c_str(), &next_type, "None\0Value\0Range X\0Range Y\0Range Z\0Range XY\0Range XZ\0Range YZ\0Range XYZ\0"))
		type = static_cast<Type>(next_type);

	switch (type) {
	case RE_EmissionVector::VALUE: ImGui::DragFloat3(name, val.ptr()); break;
	case RE_EmissionVector::RANGEX:
	{
		ImGui::DragFloat3(name, val.ptr());
		ImGui::DragFloat((tmp + " X Margin").c_str(), &margin.x);
		break; 
	}
	case RE_EmissionVector::RANGEY:
	{
		ImGui::DragFloat3(name, val.ptr());
		ImGui::DragFloat((tmp + " Y Margin").c_str(), &margin.y);
		break; 
	}
	case RE_EmissionVector::RANGEZ:
	{
		ImGui::DragFloat3(name, val.ptr());
		ImGui::DragFloat((tmp + " Z Margin").c_str(), &margin.z);
		break; 
	}
	case RE_EmissionVector::RANGEXY:
	{
		ImGui::DragFloat3(name, val.ptr());
		ImGui::DragFloat2((tmp + " XY Margin").c_str(), &margin.x);
		ImGui::DragFloat((tmp + " Y Margin").c_str(), &margin.y);
		break; 
	}
	case RE_EmissionVector::RANGEXZ:
	{
		ImGui::DragFloat3(name, val.ptr());
		float xz[2] = { margin.x, margin.z };
		if (ImGui::DragFloat2((tmp + " XZ Margin").c_str(), xz))
		{
			margin.x = xz[0];
			margin.z = xz[1];
		}
		break; 
	}
	case RE_EmissionVector::RANGEYZ:
	{
		ImGui::DragFloat3(name, val.ptr());
		ImGui::DragFloat2((tmp + " YZ Margin").c_str(), margin.ptr() + 1);
		break; 
	}
	case RE_EmissionVector::RANGEXYZ:
	{
		ImGui::DragFloat3(name, val.ptr());
		ImGui::DragFloat3((tmp + " Margin").c_str(), margin.ptr());
		break;
	}
	default: break; }
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

void RE_EmissionSingleValue::DrawEditor(const char* name)
{
	const eastl::string tmp(name);
	int next_type = static_cast<int>(type);
	if (ImGui::Combo((tmp + " type").c_str(), &next_type, "None\0Value\0Range\0"))
		type = static_cast<Type>(next_type);

	switch (type) {
	case RE_EmissionSingleValue::NONE: break;
	case RE_EmissionSingleValue::VALUE: ImGui::DragFloat(name, &val); break;
	case RE_EmissionSingleValue::RANGE: ImGui::DragFloat(name, &val); ImGui::DragFloat((tmp + " Margin").c_str(), &margin, 0.01f, 0.f); break; }
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

void RE_EmissionExternalForces::DrawEditor()
{
	int next_type = static_cast<int>(type);
	if (ImGui::Combo("External Forces", &next_type, "None\0Gravity\0Wind\0Gravity + Wind\0"))
		type = static_cast<Type>(next_type);

	switch (type) {
	case RE_EmissionExternalForces::NONE: break;
	case RE_EmissionExternalForces::GRAVITY: ImGui::DragFloat("Gravity", &gravity); break;
	case RE_EmissionExternalForces::WIND: ImGui::DragFloat3("Wind", wind.ptr()); break;
	case RE_EmissionExternalForces::WIND_GRAVITY: ImGui::DragFloat("Gravity", &gravity); ImGui::DragFloat3("Wind", wind.ptr()); break; }
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
	switch (type)
	{
	case Type::PLANE:
	{
		// Check if particle intersects or has passed plane
		float dist_to_plane = geo.plane.SignedDistance(p.position);
		if (dist_to_plane <= 0.f)
		{
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
	switch (type)
	{
	case Type::PLANE:
	{
		// Check if particle intersects or has passed plane
		float dist_to_plane = geo.plane.SignedDistance(p.position);
		if (dist_to_plane < p.col_radius)
		{
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

void RE_EmissionBoundary::DrawEditor()
{
	int tmp = static_cast<int>(type);
	if (ImGui::Combo("Boundary Type", &tmp, "None\0Plane\0Sphere\0AABB\0"))
	{
		switch (type = static_cast<Type>(tmp)) {
		case RE_EmissionBoundary::NONE: break;
		case RE_EmissionBoundary::PLANE: geo.plane = math::Plane({ 0.f, 1.f, 0.f }, 0.f); break;
		case RE_EmissionBoundary::SPHERE: geo.sphere = math::Sphere({ 0.f, 0.f, 0.f }, 10.f); break;
		case RE_EmissionBoundary::AABB: geo.box.SetFromCenterAndSize(math::vec::zero, math::vec::one * 5.f); break; }
	}

	if (type)
	{
		tmp = static_cast<int>(effect);
		if (ImGui::Combo("Boundary Effect", &tmp, "Contain\0Kill\0"))
			effect = static_cast<Effect>(tmp);

		if (effect == Effect::CONTAIN)
			ImGui::DragFloat("Boundary Restitution", &restitution, 1.f, 0.f, 100.f);

		switch (type) {
		case RE_EmissionBoundary::NONE: break;
		case RE_EmissionBoundary::PLANE:
		{
			ImGui::DragFloat("Distance to origin", &geo.plane.d, 1.f, 0.f);
			math::float2 angles = geo.plane.normal.ToSphericalCoordinatesNormalized() * RE_Math::rad_to_deg;
			if (ImGui::DragFloat2("Boundary Yaw - Pitch", angles.ptr(), 0.1f, -180.f, 180.f))
			{
				angles *= RE_Math::deg_to_rad;
				geo.plane.normal = math::vec::FromSphericalCoordinates(angles.x, angles.y);
			}

			break;
		}
		case RE_EmissionBoundary::SPHERE:
		{
			ImGui::DragFloat3("Boundary Position", geo.sphere.pos.ptr());
			ImGui::DragFloat("Boundary Radius", &geo.sphere.r, 1.f, 0.f);
			break;
		}
		case RE_EmissionBoundary::AABB:
		{
			ImGui::DragFloat3("Boundary Min", geo.box.minPoint.ptr());
			ImGui::DragFloat3("Boundary Max", geo.box.maxPoint.ptr());
			break;
		}
		}
	}
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

void RE_EmissionCollider::DrawEditor()
{
	int tmp = static_cast<int>(shape);
	if (ImGui::Combo("Collider Type", &tmp, "None\0Point\0Sphere\0"))
		shape = static_cast<Type>(tmp);

	if (shape)
	{
		ImGui::Checkbox("Inter collisions", &inter_collisions);

		mass.DrawEditor("Mass");
		restitution.DrawEditor("Restitution");

		if (shape == RE_EmissionCollider::SPHERE)
			radius.DrawEditor("Collider Radius");
	}
}

void RE_EmissionCollider::JsonDeserialize(RE_Json* node)
{
	shape = static_cast<RE_EmissionCollider::Type>(node->PullInt("Type", static_cast<int>(shape)));
	if (shape)
	{
		inter_collisions = node->PullBool("Inter collisions", inter_collisions);
		switch (shape) {
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
	node->PushInt("Type", static_cast<int>(shape));
	if (shape)
	{
		node->PushBool("Inter collisions", inter_collisions);
		switch (shape) {
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
	memcpy(&shape, cursor, size);
	cursor += size;

	if (shape)
	{
		size = sizeof(bool);
		memcpy(&inter_collisions, cursor, size);
		cursor += size;

		switch (shape) {
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
	memcpy(cursor, &shape, size);
	cursor += size;

	if (shape)
	{
		size = sizeof(bool);
		memcpy(cursor, &inter_collisions, size);
		cursor += size;

		switch (shape) {
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
	if (shape)
	{
		ret += sizeof(bool);
		switch (shape) {
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
