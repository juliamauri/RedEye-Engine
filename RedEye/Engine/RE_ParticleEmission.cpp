#include "Resource.h"

#include "RE_ParticleEmission.h"

#include "RE_Memory.h"
#include "RE_DataTypes.h"
#include "Application.h"
#include "RE_FileSystem.h"
#include "RE_FileBuffer.h"
#include "RE_Config.h"
#include "RE_Json.h"

#include "RE_ParticleEmitter.h"

void RE_ParticleEmission::LoadInMemory()
{
	if (RE_FS->Exists(GetLibraryPath())) BinaryDeserialize();
	else if (RE_FS->Exists(GetAssetPath())) { JsonDeserialize(); BinarySerialize(); }
	else if (isInternal()) ResourceContainer::inMemory = true;
	else RE_LOG_ERROR("Particle Emission %s not found on project", GetName());
}

void RE_ParticleEmission::UnloadMemory()
{
	loop = true;
	max_time = 5.f;
	start_delay = 0.0f;
	time_muliplier = 1.f;
	max_particles = 1000u;
	spawn_interval.type = RE_EmissionInterval::Type::NONE;
	spawn_mode.type = RE_EmissionSpawn::Type::FLOW;
	spawn_mode.particles_spawned = 10;
	spawn_mode.frequency = 10.f;
	initial_lifetime.type = RE_EmissionSingleValue::Type::NONE;
	initial_pos.type = RE_EmissionShape::Type::CIRCLE;
	initial_pos.geo.circle = math::Circle(math::vec::zero, { 0.f, 1.f, 0.f }, 1.f);
	initial_speed.type = RE_EmissionVector::Type::NONE;
	external_acc.type = RE_EmissionExternalForces::Type::NONE;
	external_acc.gravity = -9.81f;
	boundary.type = RE_EmissionBoundary::Type::NONE;
	collider.type = RE_EmissionCollider::Type::NONE;

	ResourceContainer::inMemory = false;
}

void RE_ParticleEmission::Import(bool keepInMemory)
{
	JsonDeserialize(true);
	BinarySerialize();
	if (!keepInMemory) UnloadMemory();
}

void RE_ParticleEmission::Save()
{
	JsonSerialize();
	BinarySerialize();
	SaveMeta();
}

void RE_ParticleEmission::ProcessMD5()
{
	JsonSerialize(true);
}

void RE_ParticleEmission::FillEmitter(RE_ParticleEmitter* to_fill)
{
	to_fill->loop = loop;
	to_fill->max_time = max_time;
	to_fill->start_delay = start_delay;
	to_fill->time_muliplier = time_muliplier;
	to_fill->max_particles = max_particles;
	to_fill->spawn_interval = spawn_interval;
	to_fill->spawn_mode = spawn_mode;
	to_fill->initial_lifetime = initial_lifetime;
	memcpy(&to_fill->initial_pos, &initial_pos, sizeof(RE_EmissionShape));
	to_fill->initial_speed = initial_speed;
	to_fill->external_acc = external_acc;
	memcpy(&to_fill->boundary, &boundary, sizeof(RE_EmissionBoundary));
	to_fill->collider = collider;
}

void RE_ParticleEmission::FillResouce(RE_ParticleEmitter* from)
{
	loop = from->loop;
	max_time = from->max_time;
	start_delay = from->start_delay;
	time_muliplier = from->time_muliplier;
	max_particles = from->max_particles;
	spawn_interval = from->spawn_interval;
	spawn_mode = from->spawn_mode;
	initial_lifetime = from->initial_lifetime;
	memcpy(&initial_pos, &from->initial_pos, sizeof(RE_EmissionShape));
	initial_speed = from->initial_speed;
	external_acc = from->external_acc;
	memcpy(&boundary, &from->boundary, sizeof(RE_EmissionBoundary));
	collider = from->collider;
}

void RE_ParticleEmission::JsonDeserialize(bool generateLibraryPath)
{
	Config emission(GetAssetPath(), RE_FS->GetZipPath());
	if (emission.Load())
	{
		RE_Json* node = emission.GetRootNode("Emission");

		loop = node->PullBool("Loop", loop);
		max_time = node->PullFloat("Max time", max_time);
		start_delay = node->PullFloat("Start Delay", start_delay);
		time_muliplier = node->PullFloat("Time Multiplier", time_muliplier);

		spawn_interval.JsonDeserialize(node->PullJObject("Interval"));
		spawn_mode.JsonDeserialize(node->PullJObject("Spawn Mode"));
		initial_lifetime.JsonDeserialize(node->PullJObject("Lifetime"));
		initial_pos.JsonDeserialize(node->PullJObject("Position"));
		initial_speed.JsonDeserialize(node->PullJObject("Speed"));
		external_acc.JsonDeserialize(node->PullJObject("Acceleration"));
		boundary.JsonDeserialize(node->PullJObject("Boundary"));
		collider.JsonDeserialize(node->PullJObject("Collider"));

		if (generateLibraryPath)
		{
			SetMD5(emission.GetMd5().c_str());
			eastl::string libraryPath("Library/Particles/");
			libraryPath += GetMD5();
			SetLibraryPath(libraryPath.c_str());
		}

		ResourceContainer::inMemory = true;
	}
}

void RE_ParticleEmission::JsonSerialize(bool onlyMD5)
{
	Config emission(GetAssetPath(), RE_FS->GetZipPath());
	RE_Json* node = emission.GetRootNode("Emission");

	node->PushBool("Loop", loop);
	node->PushFloat("Max time", max_time);
	node->PushFloat("Start Delay", start_delay);
	node->PushFloat("Time Multiplier", time_muliplier);

	spawn_interval.JsonSerialize(node->PushJObject("Interval"));
	spawn_mode.JsonSerialize(node->PushJObject("Spawn Mode"));
	initial_lifetime.JsonSerialize(node->PushJObject("Lifetime"));
	initial_pos.JsonSerialize(node->PushJObject("Position"));
	initial_speed.JsonSerialize(node->PushJObject("Speed"));
	external_acc.JsonSerialize(node->PushJObject("Acceleration"));
	boundary.JsonSerialize(node->PushJObject("Boundary"));
	collider.JsonSerialize(node->PushJObject("Collider"));

	if (!onlyMD5) emission.Save();
	SetMD5(emission.GetMd5().c_str());

	eastl::string libraryPath("Library/Particles/");
	libraryPath += GetMD5();
	SetLibraryPath(libraryPath.c_str());

	DEL(node);
}

void RE_ParticleEmission::BinaryDeserialize()
{
	RE_FileBuffer libraryFile(GetLibraryPath());
	if (libraryFile.Load())
	{
		char* cursor = libraryFile.GetBuffer();

		size_t size = sizeof(bool);
		memcpy(&loop, cursor, size);
		cursor += size;

		size = sizeof(float);
		memcpy(&max_time, cursor, size);
		cursor += size;
		memcpy(&start_delay, cursor, size);
		cursor += size;
		memcpy(&time_muliplier, cursor, size);
		cursor += size;

		size = sizeof(uint);
		memcpy(&max_particles, cursor, size);
		cursor += size;

		spawn_interval.BinaryDeserialize(cursor);
		spawn_mode.BinaryDeserialize(cursor);
		initial_lifetime.BinaryDeserialize(cursor);
		initial_pos.BinaryDeserialize(cursor);
		initial_speed.BinaryDeserialize(cursor);
		external_acc.BinaryDeserialize(cursor);
		boundary.BinaryDeserialize(cursor);
		collider.BinaryDeserialize(cursor);

		ResourceContainer::inMemory = true;
	}
}

void RE_ParticleEmission::BinarySerialize() const
{
	RE_FileBuffer libraryFile(GetLibraryPath(), RE_FS->GetZipPath());

	uint bufferSize = GetBinarySize() + 1;
	char* buffer = new char[bufferSize];
	char* cursor = buffer;

	size_t size = sizeof(bool);
	memcpy(cursor, &loop, size);
	cursor += size;

	size = sizeof(float);
	memcpy(cursor, &max_time, size);
	cursor += size;
	memcpy(cursor, &start_delay, size);
	cursor += size;
	memcpy(cursor, &time_muliplier, size);
	cursor += size;

	size = sizeof(uint);
	memcpy(cursor, &max_particles, size);
	cursor += size;

	spawn_interval.BinarySerialize(cursor);
	spawn_mode.BinarySerialize(cursor);
	initial_lifetime.BinarySerialize(cursor);
	initial_pos.BinarySerialize(cursor);
	initial_speed.BinarySerialize(cursor);
	external_acc.BinarySerialize(cursor);
	boundary.BinarySerialize(cursor);
	collider.BinarySerialize(cursor);

	char nullchar = '\0';
	memcpy(cursor, &nullchar, sizeof(char));

	libraryFile.Save(buffer, bufferSize);
	DEL_A(buffer);
}

unsigned int RE_ParticleEmission::GetBinarySize() const
{
	return sizeof(bool) + (3u * sizeof(float)) + sizeof(uint)
		+ spawn_interval.GetBinarySize()
		+ spawn_mode.GetBinarySize()
		+ initial_lifetime.GetBinarySize()
		+ initial_pos.GetBinarySize()
		+ initial_speed.GetBinarySize()
		+ external_acc.GetBinarySize()
		+ boundary.GetBinarySize()
		+ collider.GetBinarySize();
}
