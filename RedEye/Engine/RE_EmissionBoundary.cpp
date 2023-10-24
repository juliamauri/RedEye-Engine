#include "RE_EmissionBoundary.h"

#include "RE_Profiler.h"
#include "RE_Memory.h"
#include "Application.h"
#include "RE_Math.h"
#include "RE_Particle.h"
#include "RE_Json.h"

#include <ImGui/imgui.h>

bool RE_EmissionBoundary::DrawEditor()
{
	bool ret = false;
	int tmp = static_cast<int>(type);
	if (ImGui::Combo("Boundary Type", &tmp, "None\0Plane\0Sphere\0AABB\0"))
	{
		switch (type = static_cast<Type>(tmp))
		{
		case Type::PLANE: geo.plane = math::Plane({ 0.f, 1.f, 0.f }, 0.f); break;
		case Type::SPHERE: geo.sphere = math::Sphere({ 0.f, 0.f, 0.f }, 10.f); break;
		case Type::AABB: geo.box.SetFromCenterAndSize(math::vec::zero, math::vec::one * 5.f); break;
		default: break;
		}
		ret = true;
	}

	if (type == Type::NONE) return ret;

	tmp = static_cast<int>(effect);
	if (ImGui::Combo("Boundary Effect", &tmp, "Contain\0Kill\0"))
	{
		effect = static_cast<Effect>(tmp);
		ret = true;
	}

	if (effect == Effect::CONTAIN && ImGui::DragFloat("Boundary Restitution", &restitution, 1.f, 0.f, 100.f))
		ret = true;

	switch (type)
	{
	case Type::PLANE:
	{
		if (ImGui::DragFloat("Distance to origin", &geo.plane.d, 1.f, 0.f))
			ret = true;

		math::float2 angles = geo.plane.normal.ToSphericalCoordinatesNormalized() * RE_Math::rad_to_deg;
		if (ImGui::DragFloat2("Boundary Yaw - Pitch", angles.ptr(), 0.1f, -180.f, 180.f))
		{
			angles *= RE_Math::deg_to_rad;
			geo.plane.normal = math::vec::FromSphericalCoordinates(angles.x, angles.y);
			ret = true;
		}

		break;
	}
	case Type::SPHERE:
		if (ImGui::DragFloat3("Boundary Position", geo.sphere.pos.ptr()) ||
			ImGui::DragFloat("Boundary Radius", &geo.sphere.r, 1.f, 0.f))
			ret = true;
		break;
	case Type::AABB:
		if (ImGui::DragFloat3("Boundary Min", geo.box.minPoint.ptr()) ||
			ImGui::DragFloat3("Boundary Max", geo.box.maxPoint.ptr()))
			ret = true;
		break;
	default: break;
	}
	return ret;
}

bool RE_EmissionBoundary::PointCollision(RE_Particle& p) const
{
	RE_PROFILE(RE_ProfiledFunc::ParticleBoundPCol, RE_ProfiledClass::ParticleBoundary)
	switch (type)
	{
	case Type::PLANE:
	{
		// Check if particle intersects or has passed plane
		float dist_to_plane = geo.plane.SignedDistance(p.position);
		if (dist_to_plane <= 0.f)
		{
			if (effect == Effect::KILL) return false;

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
			if (effect == Effect::KILL) return false;

			// Resolve intersection
			p.position -= p.velocity.Normalized() * math::Sqrt(overlap_distance);

			// Resolve impulse only if particle not already moving away from sphere
			const math::vec impact_normal = (geo.sphere.pos - p.position).Normalized();
			float dot = p.velocity.Dot(impact_normal);
			if (dot < 0.f) p.velocity -= (p.col_restitution + restitution) * dot * impact_normal;
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
			if (effect == Effect::KILL) return false;

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
	RE_PROFILE(RE_ProfiledFunc::ParticleBoundSCol, RE_ProfiledClass::ParticleBoundary)
	switch (type)
	{
	case Type::PLANE:
	{
		// Check if particle intersects or has passed plane
		float dist_to_plane = geo.plane.SignedDistance(p.position);
		if (dist_to_plane < p.col_radius)
		{
			if (effect == Effect::KILL) return false;

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
				if (dot < 0.f) p.velocity -= (p.col_restitution + restitution) * dot * geo.plane.normal;
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
			if (effect == Effect::KILL) return false;

			// Resolve intersection
			p.position -= p.velocity.Normalized() * overlap_distance;

			// Resolve impulse only if particle not already moving away from sphere
			const math::vec impact_normal = (geo.sphere.pos - p.position).Normalized();
			float dot = p.velocity.Dot(impact_normal);
			if (dot < 0.f) p.velocity -= (p.col_restitution + restitution) * dot * impact_normal;
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
				(p.position[axis] >= geo.box.maxPoint[axis] - p.col_radius);
		}

		if (collision)
		{
			if (effect == Effect::KILL) return false;

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

void RE_EmissionBoundary::JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources) const
{
	node->Push("Type", static_cast<uint>(type));

	if (type != Type::NONE)
	{
		node->Push("Effect", static_cast<uint>(effect));

		switch (type)
		{
		case Type::PLANE:
			node->Push("Distance", geo.plane.d);
			node->PushFloatVector("Normal", geo.plane.normal);
			break;
		case Type::SPHERE:
			node->Push("Radius", geo.sphere.r);
			node->PushFloatVector("Position", geo.sphere.pos);
			break;
		case Type::AABB:
			node->PushFloatVector("Min point", geo.box.minPoint);
			node->PushFloatVector("Max point", geo.box.maxPoint);
			break;
		default: break;
		}
	}

	DEL(node)
}

void RE_EmissionBoundary::JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources)
{
	type = static_cast<Type>(node->PullUInt("Type", static_cast<uint>(type)));

	if (type != Type::NONE)
	{
		effect = static_cast<Effect>(node->PullUInt("Effect", static_cast<uint>(effect)));

		switch (type)
		{
		case Type::PLANE:
			geo.plane.d = node->PullFloat("Distance", geo.plane.d);
			geo.plane.normal = node->PullFloatVector("Normal", geo.plane.normal);
			break;
		case Type::SPHERE:
			geo.sphere.r = node->PullFloat("Radius", geo.sphere.r);
			geo.sphere.pos = node->PullFloatVector("Position", geo.sphere.pos);
			break;
		case Type::AABB:
			geo.box.minPoint = node->PullFloatVector("Min point", geo.box.minPoint);
			geo.box.maxPoint = node->PullFloatVector("Max point", geo.box.maxPoint);
			break;
		default: break;
		}
	}

	DEL(node)
}

size_t RE_EmissionBoundary::GetBinarySize() const
{
	size_t ret = sizeof(int);

	if (type != Type::NONE)
	{
		ret *= 2;
		switch (type)
		{
		case RE_EmissionBoundary::Type::PLANE: ret += sizeof(float) * 4; break;
		case RE_EmissionBoundary::Type::SPHERE: ret += sizeof(float) * 4; break;
		case RE_EmissionBoundary::Type::AABB: ret += sizeof(float) * 6; break;
		default: break;
		}
	}

	return ret;
}

void RE_EmissionBoundary::BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources) const
{
	size_t size = sizeof(ushort);
	memcpy(cursor, &type, size);
	cursor += size;

	if (type == Type::NONE) return;

	memcpy(cursor, &effect, size);
	cursor += size;

	switch (type)
	{
	case Type::PLANE:
		size = sizeof(float);
		memcpy(cursor, &geo.plane.d, size);
		cursor += size;
		size *= 3;
		memcpy(cursor, geo.plane.normal.ptr(), size);
		cursor += size;
		break;
	case Type::SPHERE:
		size = sizeof(float);
		memcpy(cursor, &geo.sphere.r, size);
		cursor += size;
		size *= 3;
		memcpy(cursor, geo.sphere.pos.ptr(), size);
		cursor += size;
		break;
	case Type::AABB:
		size = sizeof(float) * 3;
		memcpy(cursor, geo.box.minPoint.ptr(), size);
		cursor += size;
		memcpy(cursor, geo.box.maxPoint.ptr(), size);
		cursor += size;
		break;
	default: break;
	}
}

void RE_EmissionBoundary::BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources)
{
	size_t size = sizeof(ushort);
	memcpy(&type, cursor, size);
	cursor += size;

	if (type == Type::NONE) return;

	memcpy(&effect, cursor, size);
	cursor += size;

	switch (type)
	{
	case Type::PLANE:
		size = sizeof(float);
		memcpy(&geo.plane.d, cursor, size);
		cursor += size;
		size *= 3;
		memcpy(geo.plane.normal.ptr(), cursor, size);
		cursor += size;
		break;
	case Type::SPHERE:
		size = sizeof(float);
		memcpy(&geo.sphere.r, cursor, size);
		cursor += size;
		size *= 3;
		memcpy(geo.sphere.pos.ptr(), cursor, size);
		cursor += size;
		break;
	case Type::AABB:
		size = sizeof(float) * 3;
		memcpy(geo.box.minPoint.ptr(), cursor, size);
		cursor += size;
		memcpy(geo.box.maxPoint.ptr(), cursor, size);
		cursor += size;
		break;
	default: break;
	}
}
