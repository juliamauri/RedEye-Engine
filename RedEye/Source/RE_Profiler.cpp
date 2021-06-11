#include "RE_Profiler.h"

#ifdef PROFILING_ENABLED
#ifdef INTERNAL_PROFILING

#include "Application.h"
#include "MathGeoLib\include\Time\Clock.h"
#include "RapidJson\include\writer.h"
#include "RapidJson\include\stringbuffer.h"
#include <iostream>
#include <fstream>

bool ProfilingTimer::recording = RECORD_FROM_START;
unsigned long ProfilingTimer::frame = 0u;
eastl::vector<ProfilingOperation> ProfilingTimer::operations;

ProfilingTimer::ProfilingTimer(RE_ProfiledFunc f, RE_ProfiledClass c) : operation_id(0u)
{
	if (pushed = recording)
	{
		operation_id = operations.size();
		operations.push_back({ f, c, math::Clock::Tick(), 0, frame });
	}
}

ProfilingTimer::~ProfilingTimer()
{
	if (pushed)
		operations[operation_id].duration = math::Clock::Tick() - operations[operation_id].start;
}

void RE_Profiler::Start()
{
	ProfilingTimer::recording = true;
}

void RE_Profiler::Pause()
{
	ProfilingTimer::recording = false;
}

void RE_Profiler::Clear()
{
	ProfilingTimer::operations.clear();
}

void RE_Profiler::Deploy()
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
		"Get Active Shaders", "Draw Scene", "Draw Skybox", "Draw Stencil", "Draw Editor", "Draw Debug", "Draw Thumbnails",
		"Camera Raycast", "Editor Camera",
		"Thumbnail Resources", "Init Checker", "Init Shaders", "Init Water", "Init Material", "Init SkyBox",
		"Set Window Properties", "Create Window",
		"Particle Timing", "Particle Update", "Particle Spawn", "Particle Collision", "Particle BoundPCol", "Particle BoundSCol" };

	const char* class_name[PROF_CLASS_MAX] = {
		"Application", "Log", "Time", "Math", "Hardware",
		"Module Input", "Module Window",
		"Module Scene", "Camera Manager", "Primitive Manager",
		"Module Physics", "Particle Manager",  "Particle Emitter", "Particle Boundary",
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
		writer.EndObject();
	}
	writer.EndArray();
	writer.EndObject();

	std::ofstream file;
	file.open(FILE_OUT_NAME, std::ios::trunc);
	file << s.GetString();
	file.close();
}

#endif // INTERNAL_PROFILING
#endif // PROFILING_ENABLED
