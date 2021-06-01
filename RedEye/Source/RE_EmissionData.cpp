#include "RE_EmissionData.h"

#include "Application.h"
#include "RE_Math.h"
#include "RE_Particle.h"
#include "ImGui\imgui.h"

bool RE_EmissionInterval::IsActive(float &dt)
{
	switch (type)
	{
	case NONE:
	{
		is_open = true;
		break;
	}
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
		case RE_EmissionShape::CIRCLE: geo.ring = { math::Circle(math::vec::zero, { 0.f, 1.f, 0.f }, 1.f), 0.1f }; break;
		case RE_EmissionShape::RING: geo.circle = math::Circle(math::vec::zero, { 0.f, 1.f, 0.f }, 1.f); break;
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
			{
				p.position += geo.plane.normal * dist_to_plane;
				p.velocity *= 0.9999f;
			}
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
		const bool collision[6] = {
			p.position.x < geo.box.minPoint.x, // x min
			p.position.x > geo.box.maxPoint.x, // x max
			p.position.y < geo.box.minPoint.y, // y min
			p.position.y > geo.box.maxPoint.y, // y max
			p.position.z < geo.box.minPoint.z, // z min
			p.position.z > geo.box.maxPoint.z }; // z max

		if (collision[0] + collision[1] + collision[2] + collision[3] + collision[4] + collision[5] > 0)
		{
			if (effect == RE_EmissionBoundary::KILL) return false;

			math::vec impact_normal;
			if (collision[0]) // x min
			{
				p.position.x = geo.box.minPoint.x;
				float dot = p.velocity.Dot({ 1.f, 0.f, 0.f });
				if (dot < 0.f)
					p.velocity -= (p.col_restitution + restitution) * dot * math::vec(1.f, 0.f, 0.f);
			}
			if (collision[2]) // y min
			{
				p.position.y = geo.box.minPoint.y;
				float dot = p.velocity.Dot({ 0.f, 1.f, 0.f });
				if (dot < 0.f)
					p.velocity -= (p.col_restitution + restitution) * dot * math::vec(0.f, 1.f, 0.f);
			}
			if (collision[4]) // z min
			{
				p.position.z = geo.box.minPoint.z;
				float dot = p.velocity.Dot({ 0.f, 0.f, 1.f });
				if (dot < 0.f)
					p.velocity -= (p.col_restitution + restitution) * dot * math::vec(0.f, 0.f, 1.f);
			}

			if (collision[1]) // x max
			{
				p.position.x = geo.box.maxPoint.x;
				float dot = p.velocity.Dot({ -1.f, 0.f, 0.f });
				if (dot < 0.f)
					p.velocity -= (p.col_restitution + restitution) * dot * math::vec(-1.f, 0.f, 0.f);
			}
			if (collision[3]) // y max
			{
				p.position.y = geo.box.maxPoint.y;
				float dot = p.velocity.Dot({ -1.f, 0.f, 0.f });
				if (dot < 0.f)
					p.velocity -= (p.col_restitution + restitution) * dot * math::vec(0.f, -1.f, 0.f);
			}
			if (collision[5]) // z max
			{
				p.position.z = geo.box.maxPoint.z;
				float dot = p.velocity.Dot({ -1.f, 0.f, 0.f });
				if (dot < 0.f)
					p.velocity -= (p.col_restitution + restitution) * dot * math::vec(0.f, -1.f, 0.f);
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
	case Type::NONE: break;
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
			{
				p.position += geo.plane.normal * dist_to_plane;
				p.velocity *= 0.9999f;
			}
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
		const bool collision[6] = {
			(p.position.x - p.col_radius) < geo.box.minPoint.x, // x min
			(p.position.x + p.col_radius) > geo.box.maxPoint.x, // x max
			(p.position.y - p.col_radius) < geo.box.minPoint.y, // y min
			(p.position.y + p.col_radius) > geo.box.maxPoint.y, // y max
			(p.position.z - p.col_radius) < geo.box.minPoint.z, // z min
			(p.position.z + p.col_radius) > geo.box.maxPoint.z }; // z max

		if (collision[0] + collision[1] + collision[2] + collision[3] + collision[4] + collision[5] > 0)
		{
			if (effect == RE_EmissionBoundary::KILL) return false;

			math::vec impact_normal;
			if (collision[0]) // x min
			{
				p.position.x = geo.box.minPoint.x + p.col_radius;
				float dot = p.velocity.Dot({ 1.f, 0.f, 0.f });
				if (dot < 0.f)
					p.velocity -= (p.col_restitution + restitution) * dot * math::vec(1.f, 0.f, 0.f);
			}
			if (collision[2]) // y min
			{
				p.position.y = geo.box.minPoint.y + p.col_radius;
				float dot = p.velocity.Dot({ 0.f, 1.f, 0.f });
				if (dot < 0.f)
					p.velocity -= (p.col_restitution + restitution) * dot * math::vec(0.f, 1.f, 0.f);
			}
			if (collision[4]) // z min
			{
				p.position.z = geo.box.minPoint.z + p.col_radius;
				float dot = p.velocity.Dot({ 0.f, 0.f, 1.f });
				if (dot < 0.f)
					p.velocity -= (p.col_restitution + restitution) * dot * math::vec(0.f, 0.f, 1.f);
			}

			if (collision[1]) // x max
			{
				p.position.x = geo.box.maxPoint.x - p.col_radius;
				float dot = p.velocity.Dot({ -1.f, 0.f, 0.f });
				if (dot < 0.f)
					p.velocity -= (p.col_restitution + restitution) * dot * math::vec(-1.f, 0.f, 0.f);
			}
			if (collision[3]) // y max
			{
				p.position.y = geo.box.maxPoint.y - p.col_radius;
				float dot = p.velocity.Dot({ -1.f, 0.f, 0.f });
				if (dot < 0.f)
					p.velocity -= (p.col_restitution + restitution) * dot * math::vec(0.f, -1.f, 0.f);
			}
			if (collision[5]) // z max
			{
				p.position.z = geo.box.maxPoint.z - p.col_radius;
				float dot = p.velocity.Dot({ -1.f, 0.f, 0.f });
				if (dot < 0.f)
					p.velocity -= (p.col_restitution + restitution) * dot * math::vec(0.f, -1.f, 0.f);
			}
		}
		break;
	}
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
		case RE_EmissionBoundary::SPHERE: geo.sphere = math::Sphere({ 0.f, 0.f, 0.f }, 1.f); break;
		case RE_EmissionBoundary::AABB: geo.box.SetFromCenterAndSize(math::vec::zero, math::vec::one); break; }
	}

	if (type)
	{
		tmp = static_cast<int>(effect);
		if (ImGui::Combo("Boundary Effect", &tmp, "Contain\0Kill\0"))
			effect = static_cast<Effect>(tmp);

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