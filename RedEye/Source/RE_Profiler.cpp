#include "RE_Profiler.h"

#ifdef PROFILE_ACTIVE
#ifdef INTERNAL_PROFILING

#include "Application.h"

#ifdef PROFILE_CAP
#include "ModuleInput.h"
#endif // PROFILE_CAP

#include "MathGeoLib\include\Time\Clock.h"
#include "RapidJson\include\writer.h"
#include "RapidJson\include\stringbuffer.h"
#include <iostream>
#include <fstream>

unsigned int ProfilingTimer::frame = 0u;

bool RE_Profiler::enabled = false;

ProfilingTimer::ProfilingTimer(RE_ProfiledFunc f, RE_ProfiledClass c)
{
	operation_id = App->profiler->operations.size();
	App->profiler->operations.push_back({ f, c, math::Clock::Tick(), 0, frame });
}

ProfilingTimer::~ProfilingTimer()
{
	App->profiler->operations[operation_id].duration = math::Clock::Tick() - App->profiler->operations[operation_id].start;
}

RE_Profiler::RE_Profiler()
{

}

RE_Profiler::~RE_Profiler()
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

	const char* function_name[PROF_FUNC_MAX] = { "Init", "Start", "PreUpdate", "Update", "PostUpdate", "CleanUp", "Load", "Save", "Clear", "Read Asset Changes", "Dropped File", "Get Active Shaders", "Draw Scene", "Draw Skybox", "Draw Stencil", "Draw Editor", "Draw Debug", "Draw Thumbnails", "Camera Raycast", "Editor Camera", "Thumbnail Resources", "Init Checker", "Init Shaders", "Init Water", "Init Material", "Init SkyBox", "Set Window Properties", "Create Window" };
	const char* class_name[PROF_CLASS_MAX] = { "Application", "Log", "Time", "Math", "Hardware", "Module Input", "Module Window", "Module Scene", "Camera Manager", "Primitive Manager", "Module Editor", "Thumbnail Manager", "Module Render", "FBO Manager", "GL Cache", "Module Audio", "File System", "Resources Manager", "Model Importer", "Shader Importer", "ECS Importer", "Texture Importer", "Skybox Importer", "Internal Resources" };

	for (auto op : operations)
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
	file.open("Red Eye Profiling.json", std::ios::trunc);
	file << s.GetString();
	file.close();

	operations.clear();
}

void RE_Profiler::start()
{
	enabled = true;
#ifdef PROFILE_CAP
	operations.set_capacity(PROFILE_CAP);
#else
	operations.set_capacity(10000);
#endif // PROFILE_CAP
}
void RE_Profiler::dispatch()
{
	enabled = false;
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

	const char* function_name[PROF_FUNC_MAX] = { "Init", "Start", "PreUpdate", "Update", "PostUpdate", "CleanUp", "Load", "Save", "Clear", "Read Asset Changes", "Dropped File", "Get Active Shaders", "Draw Scene", "Draw Skybox", "Draw Stencil", "Draw Editor", "Draw Debug", "Draw Thumbnails", "Camera Raycast", "Editor Camera", "Thumbnail Resources", "Init Checker", "Init Shaders", "Init Water", "Init Material", "Init SkyBox", "Set Window Properties", "Create Window" };
	const char* class_name[PROF_CLASS_MAX] = { "Application", "Log", "Time", "Math", "Hardware", "Module Input", "Module Window", "Module Scene", "Camera Manager", "Primitive Manager", "Module Editor", "Thumbnail Manager", "Module Render", "FBO Manager", "GL Cache", "Module Audio", "File System", "Resources Manager", "Model Importer", "Shader Importer", "ECS Importer", "Texture Importer", "Skybox Importer", "Internal Resources" };

	for (auto op : operations)
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
	file.open("Red Eye Profiling.json", std::ios::trunc);
	file << s.GetString();
	file.close();

	operations.clear();
}

#ifdef PROFILE_CAP
void RE_Profiler::Frame()
{
	if (App->profiler->operations.size() >= PROFILE_CAP)
		RE_INPUT->Push(REQUEST_QUIT, App);
}

#endif // PROFILE_CAP

#endif // INTERNAL_PROFILING
#endif // PROFILE_ACTIVE
