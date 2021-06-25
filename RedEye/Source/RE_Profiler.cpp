#include "RE_Profiler.h"

#ifdef PROFILING_ENABLED
#ifdef INTERNAL_PROFILING

#include "Application.h"
#include "MathGeoLib\include\Time\Clock.h"
#include "RapidJson\include\writer.h"
#include "RapidJson\include\stringbuffer.h"
#include <iostream>
#include <fstream>

unsigned long long ProfilingTimer::start = 0ull;
bool ProfilingTimer::recording = RECORD_FROM_START;
unsigned long ProfilingTimer::frame = 0u;
eastl::vector<ProfilingOperation> ProfilingTimer::operations;

#if defined(PARTICLE_PHYSICS_TEST) || defined(PARTICLE_RENDER_TEST)

int ProfilingTimer::current_sim = -1;
int ProfilingTimer::wait4frame = 0;
unsigned int ProfilingTimer::update_time = 0u;
unsigned int ProfilingTimer::p_count = 0u;

#ifdef PARTICLE_PHYSICS_TEST

unsigned int ProfilingTimer::p_col_internal = 0u;
unsigned int ProfilingTimer::p_col_boundary = 0u;

#else

unsigned int ProfilingTimer::p_lights = 0u;


#endif // PARTICLE_RENDER_TEST
#endif // PARTICLE_PHYSICS_TEST || PARTICLE_RENDER_TEST

ProfilingTimer::ProfilingTimer(RE_ProfiledFunc f, RE_ProfiledClass c) : operation_id(0u)
{
#ifdef PARTICLE_PHYSICS_TEST

	if (pushed = (recording && c == PROF_ParticleManager))
	{
		operation_id = operations.size();
		operations.push_back({ f, c, math::Clock::Tick() - ProfilingTimer::start, 0ull, frame, p_count, p_col_internal, p_col_boundary });
	}

#elif defined(PARTICLE_RENDER_TEST)

	if (pushed = (recording && (f == PROF_DrawParticles || f == PROF_DrawParticlesLight)))
	{
		operation_id = operations.size();
		operations.push_back({ f, c, math::Clock::Tick() - ProfilingTimer::start, 0ull, frame, p_count, p_lights });
	}

#else

	if (pushed = recording)
	{
		operation_id = operations.size();
		operations.push_back({ f, c, math::Clock::Tick(), 0ull, frame });
	}

#endif // PARTICLE_PHYSICS_TEST
}

ProfilingTimer::~ProfilingTimer()
{
	if (pushed && operation_id < operations.size())
		operations[operation_id].duration = math::Clock::Tick() - ProfilingTimer::start - operations[operation_id].start;
}

unsigned long long ProfilingTimer::Read() const
{
	return pushed ? (math::Clock::Tick() - operations[operation_id].start) : 0ull;
}

float ProfilingTimer::ReadMs() const
{
	return pushed ? (static_cast<float>(math::Clock::Tick() - operations[operation_id].start) / 1000.f / 1000.f) : 0.f;
}

void RE_Profiler::Start() { ProfilingTimer::recording = true; }
void RE_Profiler::Pause() { ProfilingTimer::recording = false; }
void RE_Profiler::Clear() { ProfilingTimer::operations.clear(); }

void RE_Profiler::Reset()
{
	ProfilingTimer::start = math::Clock::Tick();
	ProfilingTimer::frame = 0ul;
	Clear();
}

void DumpToFile(eastl::string file_name/*, eastl::vector<ProfilingOperation> operations*/)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	writer.StartObject();

	/*/ Functions
	writer.Key("Functions");
	writer.StartArray();
	const char* function_name[PROF_FUNC_MAX] = { "Init", "Start", "PreUpdate", "Update", "PostUpdate", "CleanUp", "Load", "Save", "Clear", "Read Asset Changes", "Dropped File", "Draw Scene", "Draw Skybox", "Draw Stencil", "Draw Editor", "Draw Debug", "Camera Raycast", "Editor Camera" };
	for (unsigned short i = 0; i < PROF_FUNC_MAX; ++i)
	{
		writer.StartObject();
		writer.Key("id");
		writer.Uint(i);
		writer.Key("name");
		writer.String(function_name[i]);
		writer.EndObject();
	}
	writer.EndArray();

	// Classes
	writer.Key("Classes");
	writer.StartArray();
	const char* class_name[PROF_CLASS_MAX] = { "Application", "Log", "Time", "Math", "Hardware", "Module Input", "Module Window", "Module Scene", "Camera Manager", "Primitive Manager", "Module Editor", "Thumbnail Manager", "Module Render", "FBO Manager", "GL Cache", "Module Audio", "File System", "Resources Manager", "Model Importer", "Shader Importer", "ECS Importer", "Texture Importer", "Skybox Importer" };
	for (unsigned short i = 0; i < PROF_CLASS_MAX; ++i)
	{
		writer.StartObject();
		writer.Key("id");
		writer.Uint(i);
		writer.Key("name");
		writer.String(class_name[i]);
		writer.EndObject();
	}
	writer.EndArray();*/

	// Operations
	writer.Key("Operations");
	writer.StartArray();

	const char* function_name[PROF_FUNC_MAX] = {
		"Init", "Start", "PreUpdate", "Update", "PostUpdate", "CleanUp", "Load", "Save", "Clear",
		"Read Asset Changes", "Dropped File",
		"Get Active Shaders", "Draw Scene", "Draw Skybox", "Draw Stencil", "Draw Editor", "Draw Debug", "Draw Thumbnails", "Draw Particles", "Draw Particles Light",
		"Camera Raycast", "Editor Camera",
		"Thumbnail Resources", "Init Checker", "Init Shaders", "Init Water", "Init Material", "Init SkyBox",
		"Set Window Properties", "Create Window",
		"Particle Timing", "Particle Update", "Particle Spawn", "Particle Collision", "Particle BoundPCol", "Particle BoundSCol" };

	const char* class_name[PROF_CLASS_MAX] = {
		"Application", "Log", "Time", "Math", "Hardware",
		"Module Input", "Module Window",
		"Module Scene", "Camera Manager", "Primitive Manager",
		"Module Physics", "Particle Manager",  "Particle Emitter", "Particle Boundary", "Comp Particle Emitter"
		"Module Editor", "Thumbnail Manager",
		"Module Render", "FBO Manager", "GL Cache",
		"Module Audio",
		"File System", "Resources Manager", "Model Importer", "Shader Importer", "ECS Importer", "Texture Importer", "Skybox Importer", "Internal Resources" };

	for (auto op : ProfilingTimer::operations)
	{
		writer.StartObject();
		writer.Key("fr");
		writer.Uint(op.frame);
		writer.Key("fu");
		writer.String(function_name[op.function]);
		writer.Key("cl");
		writer.String(class_name[op.context]);
		writer.Key("du");
		writer.Uint64(op.duration);
		writer.Key("st");
		writer.Uint64(op.start);

#if defined(PARTICLE_PHYSICS_TEST) || defined(PARTICLE_RENDER_TEST)

		writer.Key("pC");
		writer.Uint(op.p_count);

#ifdef PARTICLE_PHYSICS_TEST

		writer.Key("cI");
		writer.Uint(op.p_col_internal);
		writer.Key("cB");
		writer.Uint(op.p_col_boundary);

#else
		writer.Key("lC");
		writer.Uint(op.p_lights);


#endif // PARTICLE_RENDER_TEST
#endif // PARTICLE_PHYSICS_TEST || PARTICLE_RENDER_TEST

		writer.EndObject();
	}
	writer.EndArray();
	writer.EndObject();

	std::ofstream file;
	file.open(file_name.c_str(), std::ios::trunc);
	file << s.GetString();
	file.close();
}

//#include <thread>
//static eastl::vector<std::thread> thread_pool;

void RE_Profiler::Deploy(const char* file_name)
{
	Pause();

	if (!ProfilingTimer::operations.empty())
	{
		//thread_pool.push_back(std::thread(DumpToFile, file_name, ProfilingTimer::operations));
		DumpToFile(file_name);
		Clear();
	}
}

void RE_Profiler::Exit()
{
	if (ProfilingTimer::recording) RE_Profiler::Deploy();
	else Clear();

	/*for (unsigned int i = 0u; i < thread_pool.size(); ++i) thread_pool[i].join();
	thread_pool.clear();*/
}

#endif // INTERNAL_PROFILING
#endif // PROFILING_ENABLED
