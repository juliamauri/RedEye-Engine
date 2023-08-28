#include "RE_Profiler.h"

#ifdef INTERNAL_PROFILING
#include "Application.h"
#include <RapidJson/writer.h>
#include <iostream>
#include <fstream>
#endif

#ifdef PROFILING_ENABLED

/**************************************************
******	RE_Profiler
**************************************************/

#pragma region RE_Profiler

const char* RE_Profiler::GetFunctionStr(const RE_ProfiledFunc function)
{
	switch (function)
	{
	case RE_ProfiledFunc::Init: return "Init";
	case RE_ProfiledFunc::Start: return "Start";
	case RE_ProfiledFunc::PreUpdate: return "PreUpdate";
	case RE_ProfiledFunc::Update: return "Update";
	case RE_ProfiledFunc::PostUpdate: return "PostUpdate";
	case RE_ProfiledFunc::CleanUp: return "CleanUp";
	case RE_ProfiledFunc::Load: return "Load";
	case RE_ProfiledFunc::Save: return "Save";
	case RE_ProfiledFunc::Clear: return "Clear";

	case RE_ProfiledFunc::ReadAssetChanges: return "Read Asset Changes";
	case RE_ProfiledFunc::DroppedFile: return "Dropped File";

	case RE_ProfiledFunc::GetActiveShaders: return "Get Active Shaders";
	case RE_ProfiledFunc::DrawScene: return "DrawS cene";
	case RE_ProfiledFunc::DrawSkybox: return "Draw Skybox";
	case RE_ProfiledFunc::DrawStencil: return "Draw Stencil";
	case RE_ProfiledFunc::DrawEditor: return "Draw Editor";
	case RE_ProfiledFunc::DrawDebug: return "Draw Debug";
	case RE_ProfiledFunc::DrawThumbnails: return "Draw Thumbnails";
	case RE_ProfiledFunc::DrawParticles: return "Draw Particles";
	case RE_ProfiledFunc::DrawParticlesLight: return "Camera Particles' Light";

	case RE_ProfiledFunc::CameraRaycast: return "Camera Raycast";
	case RE_ProfiledFunc::EditorCamera: return "Editor Camera";

	case RE_ProfiledFunc::ThumbnailResources: return "Thumbnail Resources";
	case RE_ProfiledFunc::InitChecker: return "Init Checker";
	case RE_ProfiledFunc::InitShaders: return "Init Shaders";
	case RE_ProfiledFunc::InitWater: return "Init Water";
	case RE_ProfiledFunc::InitMaterial: return "Init Material";
	case RE_ProfiledFunc::InitSkyBox: return "Init SkyBox";

	case RE_ProfiledFunc::SetWindowProperties: return "Set Window Properties";
	case RE_ProfiledFunc::CreateWindow: return "Create Window";

	case RE_ProfiledFunc::ParticleTiming: return "Particle Timing";
	case RE_ProfiledFunc::ParticleUpdate: return "Particle Update";
	case RE_ProfiledFunc::ParticleSpawn: return "Particle Spawn";
	case RE_ProfiledFunc::ParticleCollision: return "Particle Collision";
	case RE_ProfiledFunc::ParticleBoundPCol: return "Particle Plane Boundary Collision";
	case RE_ProfiledFunc::ParticleBoundSCol: return "Particle Sphere Boundary Collision";

	default: return "Undefined";
	}
}

#ifdef INTERNAL_PROFILING

const char* RE_Profiler::GetClassStr(const RE_ProfiledClass function)
{
	switch (function)
	{
	case RE_ProfiledClass::Application: return "Application"; // GameLogic
	case RE_ProfiledClass::Log: return "Log";
	case RE_ProfiledClass::Time: return "Time";
	case RE_ProfiledClass::Math: return "Math";
	case RE_ProfiledClass::Hardware: return "Hardware";

	case RE_ProfiledClass::ModuleInput: return "Module Input"; // Input
	case RE_ProfiledClass::ModuleWindow: return "Module Window";

	case RE_ProfiledClass::ModuleScene: return "Module Scene"; // Scene
	case RE_ProfiledClass::CameraManager: return "Camera Manager";
	case RE_ProfiledClass::PrimitiveManager: return "Primitive Manager";

	case RE_ProfiledClass::ModulePhysics: return "Module Physics"; // Physics
	case RE_ProfiledClass::ParticleManager: return "Particle Manager";
	case RE_ProfiledClass::ParticleEmitter: return "Particle Emitter";
	case RE_ProfiledClass::ParticleBoundary: return "Particle Boundary";
	case RE_ProfiledClass::CompParticleEmitter: return "Comp Particle Emitter";

	case RE_ProfiledClass::ModuleEditor: return "Module Editor"; // UI
	case RE_ProfiledClass::ThumbnailManager: return "Thumbnail Manager";

	case RE_ProfiledClass::ModuleRender: return "Module Render"; // Rendering
	case RE_ProfiledClass::RenderView: return "Render View";
	case RE_ProfiledClass::FBOManager: return "FBO Manager";
	case RE_ProfiledClass::GLCache: return "GL Cache";

	case RE_ProfiledClass::ModuleAudio: return "Module Audio"; // Audio

	case RE_ProfiledClass::FileSystem: return "File System"; // IO
	case RE_ProfiledClass::ResourcesManager: return "Resources Manager";
	case RE_ProfiledClass::ModelImporter: return "Model Importer";
	case RE_ProfiledClass::ShaderImporter: return "Shader Importer";
	case RE_ProfiledClass::ECSImporter: return "ECS Importer";
	case RE_ProfiledClass::TextureImporter: return "Texture Importer";
	case RE_ProfiledClass::SkyboxImporter: return "Skybox Importer";
	case RE_ProfiledClass::InternalResources: return "Internal Resources";

	default: return "Undefined";
	}
}

void RE_Profiler::Start() { ProfilingTimer::recording = true; }
void RE_Profiler::Pause() { ProfilingTimer::recording = false; }
void RE_Profiler::Clear() { ProfilingTimer::operations.clear(); }

void RE_Profiler::Reset()
{
	ProfilingTimer::start = math::Clock::Tick();
	ProfilingTimer::frame = 0UL;
	Clear();
}

void RE_Profiler::Exit()
{
	if (!ProfilingTimer::operations.empty())
		RE_Profiler::Deploy();
}

const char* OperationsReport()
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

	writer.StartObject();
	writer.Key("Operations");

	writer.StartArray();
	for (const auto& op : ProfilingTimer::operations)
	{
		writer.StartObject();
		writer.Key("fr");
		writer.Uint(op.frame);
		writer.Key("fu");
		writer.String(RE_Profiler::GetFunctionStr(op.function));
		writer.Key("cl");
		writer.String(RE_Profiler::GetClassStr(op.context));
		writer.Key("du");
		writer.Uint64(op.duration);
		writer.Key("st");
		writer.Uint64(op.start);

		writer.EndObject();
	}

	writer.EndArray();
	writer.EndObject();

	return buffer.GetString();
}

const char* FunctionNames()
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

	writer.StartObject();
	writer.Key("Functions");

	writer.StartArray();
	for (ushort i = 0; i < static_cast<const ushort>(RE_ProfiledFunc::END); ++i)
	{
		writer.StartObject();
		writer.Key("id");
		writer.Uint(i);
		writer.Key("name");
		writer.String(RE_Profiler::GetFunctionStr(static_cast<const RE_ProfiledFunc>(i)));
		writer.EndObject();
	}

	writer.EndArray();
	writer.EndObject();

	return buffer.GetString();
}

const char* ClassNames()
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

	writer.StartObject();
	writer.Key("Classes");

	writer.StartArray();
	for (ushort i = 0; i < static_cast<const ushort>(RE_ProfiledClass::END); ++i)
	{
		writer.StartObject();
		writer.Key("id");
		writer.Uint(i);
		writer.Key("name");
		writer.String(RE_Profiler::GetClassStr(static_cast<const RE_ProfiledClass>(i)));
		writer.EndObject();
	}

	writer.EndArray();
	writer.EndObject();

	return buffer.GetString();
}

void WriteToFile(const char* filename, const char* buffer)
{
	std::ofstream file;
	file.open(filename, std::ios::trunc);
	file << buffer;
	file.close();
}

void RE_Profiler::Deploy()
{
	Pause();
	if (!ProfilingTimer::operations.empty())
	{
		WriteToFile(
#ifdef _DEBUG
			"Debug Red Eye Profiling.json",
#else
			"Release Red Eye Profiling.json",
#endif
			OperationsReport());
		Clear();
	}

	WriteToFile("Functions Table.json", FunctionNames());
	WriteToFile("Classes Table.json", ClassNames());
}

#pragma endregion

/**************************************************
******	ProfilingOperation
**************************************************/

ProfilingOperation::ProfilingOperation(
	const RE_ProfiledFunc& function,
	const RE_ProfiledClass& context,
	const math::tick_t& start,
	const math::tick_t& duration,
	const ulong& frame)
	: function(function), context(context), start(start), duration(duration), frame(frame)
{}

bool ProfilingOperation::operator==(const ProfilingOperation& other) const
{
	return function == other.function && context == other.context && start == other.start && duration == other.duration && frame == other.frame;
}

/**************************************************
******	ProfilingTimer
**************************************************/

#pragma region ProfilingTimer

math::tick_t ProfilingTimer::start = 0;
bool ProfilingTimer::recording = RE_Profiler::RecordFromStart;
ulong ProfilingTimer::frame = 0;
eastl::vector<ProfilingOperation> ProfilingTimer::operations;

ProfilingTimer::ProfilingTimer(bool pushed, const eastl_size_t& operation_id)
	: pushed(pushed), operation_id(operation_id)
{}

ProfilingTimer::ProfilingTimer(RE_ProfiledFunc f, RE_ProfiledClass c)
{
	pushed = recording;
	if (pushed)
	{
		operation_id = operations.size();
		operations.push_back({ f, c, math::Clock::Tick(), 0, frame });
	}
}

ProfilingTimer::~ProfilingTimer()
{
	if (pushed && operation_id < operations.size())
		operations[operation_id].duration = math::Clock::Tick() - ProfilingTimer::start - operations[operation_id].start;
}

bool ProfilingTimer::operator==(const ProfilingTimer& other) const
{
	return operation_id == other.operation_id;
}


math::tick_t ProfilingTimer::Read() const
{
	return pushed ? (math::Clock::Tick() - operations[operation_id].start) : 0;
}

float ProfilingTimer::ReadMs() const
{
	return pushed ? (static_cast<float>(math::Clock::Tick() - operations[operation_id].start) / 1000.f / 1000.f) : 0.f;
}
#pragma endregion

#endif // INTERNAL_PROFILING
#endif // PROFILING_ENABLED