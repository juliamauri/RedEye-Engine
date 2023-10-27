#include "RE_PR_Light.h"

#include "RE_Memory.h"
#include "RE_Random.h"
#include "RE_Json.h"
#include "RE_ParticleManager.h"

#include <ImGui/imgui.h>
#include <EASTL/vector.h>

bool RE_PR_Light::DrawEditor(P_UID id)
{
	bool simulation_edited = false;
	int tmp = static_cast<int>(type);
	if (ImGui::Combo("Light Mode", &tmp, "None\0Unique\0Per Particle\0"))
	{
		switch (type = static_cast<Type>(tmp))
		{
		case Type::UNIQUE:
		{
			eastl::vector<RE_Particle> particles;
			if (!RE_ParticleManager::GetParticles(id, particles))
				break;

			color = GetColor();
			intensity = GetIntensity();
			specular = GetSpecular();

			for (auto& p : particles)
			{
				p.lightColor = color;
				p.intensity = intensity;
				p.specular = specular;
			}

			break;
		}
		case Type::PER_PARTICLE:
		{
			eastl::vector<RE_Particle> particles;
			if (!RE_ParticleManager::GetParticles(id, particles))
				break;

			for (auto& p : particles)
			{
				p.lightColor = GetColor();
				p.intensity = GetIntensity();
				p.specular = GetSpecular();
			}

			break;
		}
		default: break;
		}

		simulation_edited = true;
	}

	switch (type)
	{
	case Type::UNIQUE:
	{
		eastl::vector<RE_Particle> particles;
		if (!RE_ParticleManager::GetParticles(id, particles))
			break;

		if (ImGui::ColorEdit3("Light Color", color.ptr()) ||
			ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 50.0f, "%.2f") ||
			ImGui::DragFloat("Specular", &specular, 0.01f, 0.f, 1.f, "%.2f"))
			simulation_edited = true;

		ImGui::Separator();
		if (ImGui::DragFloat("Constant", &constant, 0.01f, 0.001f, 5.0f, "%.2f") ||
			ImGui::DragFloat("Linear", &linear, 0.001f, 0.001f, 5.0f, "%.3f") ||
			ImGui::DragFloat("Quadratic", &quadratic, 0.001f, 0.001f, 5.0f, "%.3f"))
			simulation_edited = true;

		break;
	}
	case Type::PER_PARTICLE:
	{
		eastl::vector<RE_Particle> particles;
		if (!RE_ParticleManager::GetParticles(id, particles))
			break;

		if (ImGui::Checkbox("Random Color", &random_color))
		{
			for (auto& p : particles) p.lightColor = GetColor();
			simulation_edited = true;
		}

		if (!random_color && ImGui::ColorEdit3("Light Color", color.ptr()))
		{
			for (auto& p : particles) p.lightColor = color;
			simulation_edited = true;
		}

		if (ImGui::Checkbox("Random Intensity", &random_i))
		{
			for (auto& p : particles) p.intensity = GetIntensity();
			simulation_edited = true;
		}

		if (random_i)
		{
			bool update_sim = false;
			update_sim |= ImGui::DragFloat("Intensity Min", &intensity, 0.01f, 0.0f, intensity_max, "%.2f");
			update_sim |= ImGui::DragFloat("Intensity Max", &intensity_max, 0.01f, intensity, 50.f, "%.2f");
			if (update_sim)
			{
				for (auto& p : particles) p.intensity = GetIntensity();
				simulation_edited = true;
			}
		}
		else if (ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 50.0f, "%.2f"))
		{
			for (auto& p : particles) p.intensity = intensity;
			simulation_edited = true;
		}

		if (ImGui::Checkbox("Random Specular", &random_s))
		{
			for (auto& p : particles) p.specular = GetSpecular();
			simulation_edited = true;
		}

		if (random_s)
		{
			bool update_sim = false;
			update_sim |= ImGui::DragFloat("Specular Min", &specular, 0.01f, 0.0f, specular_max, "%.2f");
			update_sim |= ImGui::DragFloat("Specular Max", &specular_max, 0.01f, specular, 10.f, "%.2f");

			if (update_sim)
			{
				for (auto& p : particles) p.specular = GetSpecular();
				simulation_edited = true;
			}
		}
		else if (ImGui::DragFloat("Specular", &specular, 0.01f, 0.f, 1.f, "%.2f"))
		{
			for (auto& p : particles) p.specular = specular;
			simulation_edited = true;
		}

		ImGui::Separator();

		if (ImGui::DragFloat("Constant", &constant, 0.01f, 0.001f, 5.0f, "%.2f")) simulation_edited = true;
		if (ImGui::DragFloat("Linear", &linear, 0.001f, 0.001f, 5.0f, "%.3f")) simulation_edited = true;
		if (ImGui::DragFloat("Quadratic", &quadratic, 0.001f, 0.001f, 5.0f, "%.3f")) simulation_edited = true;

		break;
	}
	default: break;
	}

	return simulation_edited;
}

math::vec RE_PR_Light::GetColor() const { return random_color ? RE_Random::RandomVec() : color; }
float RE_PR_Light::GetIntensity() const { return random_i ? intensity + (RE_Random::RandomF() * (intensity_max - intensity)) : intensity; }
float RE_PR_Light::GetSpecular() const { return random_s ? specular + (RE_Random::RandomF() * (specular_max - specular)) : specular; }
math::vec RE_PR_Light::GetQuadraticValues() const { return { constant, linear, quadratic }; }

void RE_PR_Light::JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources) const
{
	node->Push("Type", static_cast<uint>(type));

	switch (type)
	{
	case Type::UNIQUE:
	{
		node->PushFloatVector("Color", color);
		node->Push("Intensity", intensity);
		node->Push("Specular", specular);

		node->Push("Constant", constant);
		node->Push("Linear", linear);
		node->Push("Quadratic", quadratic);

		break;
	}
	case Type::PER_PARTICLE:
	{
		node->Push("Random Color", random_color);
		if (!random_color) node->PushFloatVector("Color", color);

		node->Push("Random Intensity", random_i);
		node->Push("Intensity", intensity);
		if (random_i) node->Push("Intensity Max", intensity_max);

		node->Push("Random Specular", random_s);
		node->Push("Specular", specular);
		if (random_s) node->Push("Specular Max", specular_max);

		node->Push("Constant", constant);
		node->Push("Linear", linear);
		node->Push("Quadratic", quadratic);

		break;
	}
	default: break;
	}

	DEL(node)
}

void RE_PR_Light::JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources)
{
	type = static_cast<RE_PR_Light::Type>(node->PullUInt("Type", static_cast<uint>(type)));

	switch (type)
	{
	case Type::UNIQUE:
	{
		color = node->PullFloatVector("Color", color);
		intensity = node->PullFloat("Intensity", intensity);
		specular = node->PullFloat("Specular", specular);

		constant = node->PullFloat("Constant", constant);
		linear = node->PullFloat("Linear", linear);
		quadratic = node->PullFloat("Quadratic", quadratic);

		break;
	}
	case Type::PER_PARTICLE:
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
	default: break;
	}

	DEL(node)
}

size_t RE_PR_Light::GetBinarySize() const
{
	size_t ret = sizeof(Type);

	switch (type)
	{
	case Type::UNIQUE:
		ret += sizeof(float) * 8;
		break;
	case Type::PER_PARTICLE:
	{
		ret += sizeof(bool) * 3;
		ret += sizeof(float) * 5;

		if (!random_color) ret += sizeof(float) * 3;
		if (!random_i) ret += sizeof(float);
		if (!random_s) ret += sizeof(float);

		break;
	}
	default: break;
	}

	return ret;
}

void RE_PR_Light::BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources) const
{
	size_t size = sizeof(Type);
	memcpy(cursor, &type, size);
	cursor += size;

	switch (type)
	{
	case Type::UNIQUE:
	{
		size = sizeof(float);
		memcpy(cursor, color.ptr(), size * 3);
		cursor += size * 3;
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
	case Type::PER_PARTICLE:
	{
		size = sizeof(bool);
		memcpy(cursor, &random_color, size);
		cursor += size;
		if (!random_color)
		{
			size = sizeof(float) * 3;
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

void RE_PR_Light::BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources)
{
	size_t size = sizeof(Type);
	memcpy(&type, cursor, size);
	cursor += size;

	switch (type)
	{
	case Type::UNIQUE:
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
	case Type::PER_PARTICLE:
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
	default: break;
	}
}