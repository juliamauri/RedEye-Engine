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
	case CIRCLE: return geo.circle.GetPoint(RE_MATH->RandomF() * 6.283185307f);
	case BOX:
		return { geo.box[0].x + (RE_MATH->RandomF() * (geo.box[1].x - geo.box[0].x)),
			geo.box[0].y + (RE_MATH->RandomF() * (geo.box[1].y - geo.box[0].y)),
			geo.box[0].z + (RE_MATH->RandomF() * (geo.box[1].z - geo.box[0].z)) };
	case SPHERE: return geo.sphere.pos + ((RE_MATH->RandomF() * geo.sphere.r) * RE_MATH->RandomNDir());
	case HOLLOW_SPHERE:
		float rand_radius = geo.hollow_sphere.first.r + (geo.hollow_sphere.second + RE_MATH->RandomFN());
		return geo.hollow_sphere.first.pos + (rand_radius * RE_MATH->RandomNDir());
	}
}

void RE_EmissionShape::DrawEditor()
{
	int next_shape = static_cast<int>(shape);
	if (ImGui::Combo("Emissor Shape", &next_shape, "Point\0Box\0Cirle\0Sphere\Hollow Sphere\0"))
	{
		switch (shape = static_cast<Type>(next_shape)) {
		case RE_EmissionShape::POINT: geo.point = math::vec::zero; break;
		case RE_EmissionShape::CIRCLE: geo.circle = math::Circle(math::vec::zero, { 0.f, 1.f, 0.f }, 1.f); break;
		case RE_EmissionShape::BOX: geo.box[0] = math::vec::one * -0.5f; geo.box[1] = math::vec::one * 0.5f; break;
		case RE_EmissionShape::SPHERE: geo.sphere = math::Sphere(math::vec::zero, 1.f); break;
		case RE_EmissionShape::HOLLOW_SPHERE: geo.hollow_sphere = { math::Sphere(math::vec::zero, 1.f), 0.8f }; break; }
	}

	switch (shape)
	{
	case RE_EmissionShape::POINT:
	{
		ImGui::DragFloat3("Position", geo.point.ptr());
		break;
	}
	case RE_EmissionShape::CIRCLE:
	{
		ImGui::DragFloat3("Position", geo.circle.pos.ptr());
		ImGui::DragFloat("Radius", &geo.circle.r, 1.f, 0.f);

		float yaw = math::Atan(-geo.circle.normal.x / (geo.circle.normal.y));
		float pitch = math::Atan(math::Sqrt((geo.circle.normal.x * geo.circle.normal.x) + (geo.circle.normal.y * geo.circle.normal.y)) / geo.circle.normal.z);
		if (ImGui::DragFloat("Yaw", &yaw) || ImGui::DragFloat("Pitch", &pitch))
		{
			const float cos_pitch = math::Cos(pitch);
			geo.circle.normal = math::vec(cos_pitch * math::Cos(yaw), math::Sin(pitch), cos_pitch * math::Sin(-yaw)).Normalized();
		}
		break;
	}
	case RE_EmissionShape::BOX:
	{
		ImGui::DragFloat3("Min Point", geo.box[0].ptr());
		ImGui::DragFloat3("Max Point", geo.box[1].ptr());
		break;
	}
	case RE_EmissionShape::SPHERE:
	{
		ImGui::DragFloat3("Position", geo.sphere.pos.ptr());
		ImGui::DragFloat("Radius", &geo.sphere.r, 1.f, 0.f);
		break;
	}
	case RE_EmissionShape::HOLLOW_SPHERE:
	{
		ImGui::DragFloat3("Position", geo.hollow_sphere.first.pos.ptr());
		ImGui::DragFloat("Radius", &geo.hollow_sphere.first.r, 1.f, 0.f);
		ImGui::DragFloat("Ring Radius", &geo.hollow_sphere.second, 1.f, 0.f);
		break;
	}
	}
}

math::vec RE_EmissionVector::GetSpeed() const
{
	switch (type) {
	case NONE: return math::vec::zero;
	case VALUE: return val;
	case RANGEX: return { val.x + (RE_MATH->RandomFN() * dt.x), val.y, val.z};
	case RANGEY: return { val.x, val.y + (RE_MATH->RandomFN() * dt.y), val.z };
	case RANGEZ: return { val.x, val.y, val.z + (RE_MATH->RandomFN() * dt.z) };
	case RANGEXY: return { val.x + (RE_MATH->RandomFN() * dt.x), val.y + (RE_MATH->RandomFN() * dt.y), val.z };
	case RANGEXZ: return { val.x + (RE_MATH->RandomFN() * dt.x), val.y, val.z + (RE_MATH->RandomFN() * dt.z) };
	case RANGEYZ: return { val.x, val.y + (RE_MATH->RandomFN() * dt.y), val.z + (RE_MATH->RandomFN() * dt.z) };
	case RANGEXYZ: return { val.x + (RE_MATH->RandomFN() * dt.x), val.y + (RE_MATH->RandomFN() * dt.y), val.z + (RE_MATH->RandomFN() * dt.z) }; }
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
		ImGui::DragFloat("X variance", &dt.x);
		break; 
	}
	case RE_EmissionVector::RANGEY:
	{
		ImGui::DragFloat3(name, val.ptr());
		ImGui::DragFloat("Y variance", &dt.y);
		break; 
	}
	case RE_EmissionVector::RANGEZ:
	{
		ImGui::DragFloat3(name, val.ptr());
		ImGui::DragFloat("Z variance", &dt.z);
		break; 
	}
	case RE_EmissionVector::RANGEXY:
	{
		ImGui::DragFloat3(name, val.ptr());
		ImGui::DragFloat("X variance", &dt.x);
		ImGui::DragFloat("Y variance", &dt.y);
		break; 
	}
	case RE_EmissionVector::RANGEXZ:
	{
		ImGui::DragFloat3(name, val.ptr());
		ImGui::DragFloat("X variance", &dt.x);
		ImGui::DragFloat("Z variance", &dt.z);
		break; 
	}
	case RE_EmissionVector::RANGEYZ:
	{
		ImGui::DragFloat3(name, val.ptr());
		ImGui::DragFloat("Y variance", &dt.y);
		ImGui::DragFloat("Z variance", &dt.z);
		break; 
	}
	case RE_EmissionVector::RANGEXYZ:
	{
		ImGui::DragFloat3(name, val.ptr());
		ImGui::DragFloat("X variance", &dt.x);
		ImGui::DragFloat("Y variance", &dt.y);
		ImGui::DragFloat("Z variance", &dt.z);
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
	case RE_EmissionExternalForces::WIND_GRAVITY: ImGui::DragFloat("Gravity", &gravity); ImGui::DragFloat3("Wind", wind.ptr()); break;
	default: break;
	}
}

bool RE_EmissionBoundary::ParticleCollision(RE_Particle& p) const
{
	switch (type)
	{
	case Type::NONE: break;
	case Type::PLANE:
	{
		// Check if particle intersects or has passed plane
		float dist_to_plane = data.plane.SignedDistance(p.position);
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
					data.plane.normal,
					data.plane.d + p.col_radius,
					p.position,
					norm_speed,
					dist_to_col))
				{
					p.position += norm_speed * dist_to_col;

					// Resolve impulse if particle not already moving away from plane
					float dot = p.velocity.Dot(data.plane.normal);
					if (dot < 0.f)
						p.velocity -= (p.col_restitution + restitution) * dot * data.plane.normal;
				}
				break;
			}
			case RE_EmissionBoundary::KILL: return false;
			case RE_EmissionBoundary::CLAMP: break;
			}
		}

		break;
	}
	case Type::SPHERE:
	{

		if (data.sphere.Distance(math::Sphere(p.position, p.col_radius)) > 0.f)
		{
			switch (effect)
			{
			case RE_EmissionBoundary::CONTAIN:
				break;
			case RE_EmissionBoundary::KILL: return false;
			case RE_EmissionBoundary::CLAMP: break;
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
	if (ImGui::Combo("Boundary Type", &tmp, "None\0Plane\0Box - WIP\0Sphere - WIP\0"))
	{
		switch (type = static_cast<Type>(tmp)) {
		case RE_EmissionBoundary::NONE: break;
		case RE_EmissionBoundary::PLANE: data.plane = math::Plane({ 0.f, 1.f, 0.f }, 0.f); break;
		case RE_EmissionBoundary::SPHERE: data.sphere = math::Sphere({ 0.f, 0.f, 0.f }, 1.f); break;
		case RE_EmissionBoundary::BOX: data.box[0] = -math::vec::one; data.box[0] = math::vec::one; break; }
	}

	tmp = static_cast<int>(effect);
	if (ImGui::Combo("Boundary Effect", &tmp, "Contain\0Kill\0Clamp - WIP\0"))
		effect = static_cast<Effect>(tmp);

	switch (type) {
	case RE_EmissionBoundary::NONE: break;
	case RE_EmissionBoundary::PLANE:
	{
		ImGui::DragFloat("Distance", &data.plane.d, 1.f, 0.f);

		float yaw = math::Atan(-data.plane.normal.x / (data.plane.normal.y));
		float pitch = math::Atan(math::Sqrt((data.plane.normal.x * data.plane.normal.x) + (data.plane.normal.y * data.plane.normal.y)) / data.plane.normal.z);
		if (ImGui::DragFloat("Yaw", &yaw) || ImGui::DragFloat("Pitch", &pitch))
		{
			const float cos_pitch = math::Cos(pitch);
			data.plane.normal = math::vec(cos_pitch * math::Cos(yaw), math::Sin(pitch), cos_pitch * math::Sin(-yaw)).Normalized();
		}

		break;
	}
	case RE_EmissionBoundary::SPHERE:
	{
		ImGui::DragFloat3("Position", data.sphere.pos.ptr());
		ImGui::DragFloat("Radius", &data.sphere.r, 1.f, 0.f);
		break; 
	}
	case RE_EmissionBoundary::BOX:
	{
		ImGui::DragFloat3("Min Point", data.box[0].ptr());
		ImGui::DragFloat3("Max Point", data.box[1].ptr());
		break; 
	}
	}
}
