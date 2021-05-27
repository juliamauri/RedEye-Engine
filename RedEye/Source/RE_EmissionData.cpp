#include "RE_EmissionData.h"

#include "Application.h"
#include "RE_Math.h"
#include "RE_Particle.h"
#include "ImGui\imgui.h"

math::vec RE_EmissionShape::GetPosition() const
{
	switch (shape)
	{
	case POINT: return geo.point;
	case CIRCLE: return geo.circle.GetPoint(RE_MATH->RandomF() * RE_Math::pi_x2);
	case RING: return geo.ring.first.GetPoint(RE_MATH->RandomF() * RE_Math::pi_x2) + (RE_MATH->RandomNDir() * geo.ring.second);
	case BOX:
		return { geo.box[0].x + (RE_MATH->RandomFN() * geo.box[1].x),
			geo.box[0].y + (RE_MATH->RandomFN() * geo.box[1].y),
			geo.box[0].z + (RE_MATH->RandomFN() * geo.box[1].z) };
	case SPHERE: return geo.sphere.pos + ((RE_MATH->RandomF() * geo.sphere.r) * RE_MATH->RandomNDir());
	case HOLLOW_SPHERE:
		float rand_radius = geo.hollow_sphere.first.r + (geo.hollow_sphere.second + RE_MATH->RandomFN());
		return geo.hollow_sphere.first.pos + (rand_radius * RE_MATH->RandomNDir());
	}
}

void RE_EmissionShape::DrawEditor()
{
	int next_shape = static_cast<int>(shape);
	if (ImGui::Combo("Emissor Shape", &next_shape, "Point\0Box\0Cirle\0Ring\0Sphere\0Hollow Sphere\0"))
	{
		switch (shape = static_cast<Type>(next_shape)) {
		case RE_EmissionShape::POINT: geo.point = math::vec::zero; break;
		case RE_EmissionShape::CIRCLE: geo.ring = { math::Circle(math::vec::zero, { 0.f, 1.f, 0.f }, 1.f), 0.1f }; break;
		case RE_EmissionShape::RING: geo.circle = math::Circle(math::vec::zero, { 0.f, 1.f, 0.f }, 1.f); break;
		case RE_EmissionShape::BOX: geo.box[0] = math::vec::one * -0.5f; geo.box[1] = math::vec::one * 0.5f; break;
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
		ImGui::DragFloat3("Starting Pos", geo.circle.pos.ptr());
		ImGui::DragFloat("Radius", &geo.circle.r, 1.f, 0.f);

		
		math::float2 angles = geo.circle.normal.ToSphericalCoordinatesNormalized() * RE_Math::rad_to_deg;
		if (ImGui::DragFloat2("Yaw - Pitch", angles.ptr(), 0.1f, -180.f, 180.f))
		{
			angles *= RE_Math::deg_to_rad;
			geo.circle.normal = math::vec::FromSphericalCoordinates(angles.x, angles.y);
		}
		break;
	}
	case RE_EmissionShape::RING:
	{
		ImGui::DragFloat3("Starting Pos", geo.ring.first.pos.ptr());
		ImGui::DragFloat("Radius", &geo.ring.first.r, 1.f, 0.f);
		ImGui::DragFloat("Inner Radius", &geo.ring.second, 1.f, 0.f, geo.ring.first.r);

		math::float2 angles = geo.ring.first.normal.ToSphericalCoordinatesNormalized() * RE_Math::rad_to_deg;
		if (ImGui::DragFloat2("Yaw - Pitch", angles.ptr(), 0.1f, -180.f, 180.f))
		{
			angles *= RE_Math::deg_to_rad;
			geo.ring.first.normal = math::vec::FromSphericalCoordinates(angles.x, angles.y);
		}
		break;
	}
	case RE_EmissionShape::BOX:
	{
		ImGui::DragFloat3("Center", geo.box[0].ptr());
		ImGui::DragFloat3("Margin", geo.box[1].ptr());
		break;
	}
	case RE_EmissionShape::SPHERE:
	{
		ImGui::DragFloat3("Starting Pos", geo.sphere.pos.ptr());
		ImGui::DragFloat("Radius", &geo.sphere.r, 1.f, 0.f);
		break;
	}
	case RE_EmissionShape::HOLLOW_SPHERE:
	{
		ImGui::DragFloat3("Starting Pos", geo.hollow_sphere.first.pos.ptr());
		ImGui::DragFloat("Radius", &geo.hollow_sphere.first.r, 1.f, 0.f);
		ImGui::DragFloat("Inner Radius", &geo.hollow_sphere.second, 1.f, 0.f, geo.hollow_sphere.first.r);
		break;
	}
	}
}

math::vec RE_EmissionVector::GetSpeed() const
{
	switch (type) {
	case NONE: return math::vec::zero;
	case VALUE: return val;
	case RANGEX: return { val.x + (RE_MATH->RandomFN() * margin.x), val.y, val.z};
	case RANGEY: return { val.x, val.y + (RE_MATH->RandomFN() * margin.y), val.z };
	case RANGEZ: return { val.x, val.y, val.z + (RE_MATH->RandomFN() * margin.z) };
	case RANGEXY: return { val.x + (RE_MATH->RandomFN() * margin.x), val.y + (RE_MATH->RandomFN() * margin.y), val.z };
	case RANGEXZ: return { val.x + (RE_MATH->RandomFN() * margin.x), val.y, val.z + (RE_MATH->RandomFN() * margin.z) };
	case RANGEYZ: return { val.x, val.y + (RE_MATH->RandomFN() * margin.y), val.z + (RE_MATH->RandomFN() * margin.z) };
	case RANGEXYZ: return val.Mul(margin); }
}

void RE_EmissionVector::DrawEditor(const char* name)
{
	int next_type = static_cast<int>(type);
	if (ImGui::Combo((eastl::string(name) + " type").c_str(), &next_type, "None\0Value\0Range X\0Range Y\0Range Z\0Range XY\0Range XZ\0Range YZ\0Range XYZ\0"))
		type = static_cast<Type>(next_type);

	switch (type) {
	case RE_EmissionVector::VALUE: ImGui::DragFloat3(name, val.ptr()); break;
	case RE_EmissionVector::RANGEX:
	{
		ImGui::DragFloat3(name, val.ptr());
		ImGui::DragFloat("X Margin", &margin.x);
		break; 
	}
	case RE_EmissionVector::RANGEY:
	{
		ImGui::DragFloat3(name, val.ptr());
		ImGui::DragFloat("Y Margin", &margin.y);
		break; 
	}
	case RE_EmissionVector::RANGEZ:
	{
		ImGui::DragFloat3(name, val.ptr());
		ImGui::DragFloat("Z Margin", &margin.z);
		break; 
	}
	case RE_EmissionVector::RANGEXY:
	{
		ImGui::DragFloat3(name, val.ptr());
		ImGui::DragFloat("X Margin", &margin.x);
		ImGui::DragFloat("Y Margin", &margin.y);
		break; 
	}
	case RE_EmissionVector::RANGEXZ:
	{
		ImGui::DragFloat3(name, val.ptr());
		ImGui::DragFloat("X Margin", &margin.x);
		ImGui::DragFloat("Z Margin", &margin.z);
		break; 
	}
	case RE_EmissionVector::RANGEYZ:
	{
		ImGui::DragFloat3(name, val.ptr());
		ImGui::DragFloat("Y Margin", &margin.y);
		ImGui::DragFloat("Z Margin", &margin.z);
		break; 
	}
	case RE_EmissionVector::RANGEXYZ:
	{
		ImGui::DragFloat3(name, val.ptr());
		ImGui::DragFloat3("Margin", margin.ptr());
		break;
	}
	default: break; }
}

float RE_EmissionSingleValue::GetValue() const
{
	switch (type) {
	case NONE: return 0.f;
	case VALUE: return val;
	case RANGE: return val + (RE_MATH->RandomFN() * dt); }
}

void RE_EmissionSingleValue::DrawEditor(const char* name)
{
	int next_type = static_cast<int>(type);
	if (ImGui::Combo((eastl::string(name) + " type").c_str(), &next_type, "None\0Value\0Range\0"))
		type = static_cast<Type>(next_type);

	switch (type) {
	case RE_EmissionSingleValue::NONE: break;
	case RE_EmissionSingleValue::VALUE: ImGui::DragFloat(name, &val); break;
	case RE_EmissionSingleValue::RANGE: ImGui::DragFloat(name, &val); ImGui::DragFloat("Variance", &dt); break;
	default: break; }
}

math::vec RE_EmissionExternalForces::GetAcceleration() const
{
	switch (type) {
	case RE_EmissionExternalForces::NONE: return math::vec::zero; break;
	case RE_EmissionExternalForces::GRAVITY: return math::vec(0.f, gravity, 0.f);
	case RE_EmissionExternalForces::WIND: return wind;
	case RE_EmissionExternalForces::WIND_GRAVITY: return math::vec(wind.x, wind.y + gravity, wind.z);
	}
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

bool RE_EmissionBoundary::ParticleCollision(RE_Particle& p) const
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
			switch (effect)
			{
			case RE_EmissionBoundary::CONTAIN:
			{
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
				break;
			}
			case RE_EmissionBoundary::KILL: return false;
			}
		}

		break;
	}
	case Type::SPHERE:
	{
		float overlap_distance = p.position.Distance(geo.sphere.pos) + p.col_radius - geo.sphere.r;
		if (overlap_distance > 0.f)
		{
			switch (effect)
			{
			case RE_EmissionBoundary::CONTAIN:
			{
				// Resolve intersection
				p.position -= p.velocity.Normalized() * overlap_distance;

				// Resolve impulse only if particle not already moving away from sphere
				const math::vec impact_normal = (geo.sphere.pos - p.position).Normalized();
				float dot = p.velocity.Dot(impact_normal);
				if (dot < 0.f)
					p.velocity -= (p.col_restitution + restitution) * dot * impact_normal;

				break;
			}
			case RE_EmissionBoundary::KILL: return false;
			}
		}
		
		break;
	}
	case Type::BOX: break;
	}

	return true;
}

void RE_EmissionBoundary::DrawEditor()
{
	int tmp = static_cast<int>(type);
	if (ImGui::Combo("Boundary Type", &tmp, "None\0Plane\0Sphere - WIP\0Box - WIP\0"))
	{
		switch (type = static_cast<Type>(tmp)) {
		case RE_EmissionBoundary::NONE: break;
		case RE_EmissionBoundary::PLANE: geo.plane = math::Plane({ 0.f, 1.f, 0.f }, 0.f); break;
		case RE_EmissionBoundary::SPHERE: geo.sphere = math::Sphere({ 0.f, 0.f, 0.f }, 1.f); break;
		case RE_EmissionBoundary::BOX: geo.box[0] = -math::vec::one; geo.box[1] = math::vec::one; break; }
	}

	tmp = static_cast<int>(effect);
	if (ImGui::Combo("Boundary Effect", &tmp, "Contain\0Kill\0"))
		effect = static_cast<Effect>(tmp);

	switch (type) {
	case RE_EmissionBoundary::NONE: break;
	case RE_EmissionBoundary::PLANE:
	{
		ImGui::DragFloat("Distance", &geo.plane.d, 1.f, 0.f);

		math::float2 angles = geo.plane.normal.ToSphericalCoordinatesNormalized() * RE_Math::rad_to_deg;
		if (ImGui::DragFloat2("Yaw - Pitch", angles.ptr(), 0.1f, -180.f, 180.f))
		{
			angles *= RE_Math::deg_to_rad;
			geo.plane.normal = math::vec::FromSphericalCoordinates(angles.x, angles.y);
		}

		break;
	}
	case RE_EmissionBoundary::SPHERE:
	{
		ImGui::DragFloat3("Position", geo.sphere.pos.ptr());
		ImGui::DragFloat("Radius", &geo.sphere.r, 1.f, 0.f);
		break; 
	}
	case RE_EmissionBoundary::BOX:
	{
		ImGui::DragFloat3("Min Point", geo.box[0].ptr());
		ImGui::DragFloat3("Max Point", geo.box[1].ptr());
		break; 
	}
	}
}
