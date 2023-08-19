#ifndef __RE_PROFILER__
#define __RE_PROFILER__

#define PROFILING_ENABLED // undefine to disable any profiling methods
#define INTERNAL_PROFILING // undefine to use Optick Profiling
#define RECORD_FROM_START true

enum RE_ProfiledFunc : unsigned short
{
	PROF_Init, // Modules
	PROF_Start,
	PROF_PreUpdate,
	PROF_Update,
	PROF_PostUpdate,
	PROF_CleanUp,
	PROF_Load,
	PROF_Save,
	PROF_Clear,

	PROF_ReadAssetChanges, // File System
	PROF_DroppedFile,

	PROF_GetActiveShaders, // Rendering
	PROF_DrawScene,
	PROF_DrawSkybox,
	PROF_DrawStencil,
	PROF_DrawEditor,
	PROF_DrawDebug,
	PROF_DrawThumbnails,
	PROF_DrawParticles,
	PROF_DrawParticlesLight,

	PROF_CameraRaycast, // Cameras
	PROF_EditorCamera,

	PROF_ThumbnailResources, // Resources
	PROF_InitChecker,
	PROF_InitShaders,
	PROF_InitWater,
	PROF_InitMaterial,
	PROF_InitSkyBox,

	PROF_SetWindowProperties, // Window
	PROF_CreateWindow,

	PROF_ParticleTiming, // Particles
	PROF_ParticleUpdate,
	PROF_ParticleSpawn,
	PROF_ParticleCollision,
	PROF_ParticleBoundPCol,
	PROF_ParticleBoundSCol,

	PROF_FUNC_MAX
};

enum RE_ProfiledClass : unsigned short
{
	PROF_Application, // GameLogic
	PROF_Log,
	PROF_Time,
	PROF_Math,
	PROF_Hardware,

	PROF_ModuleInput, // Input
	PROF_ModuleWindow,

	PROF_ModuleScene, // Scene
	PROF_CameraManager,
	PROF_PrimitiveManager,

	PROF_ModulePhysics, // Physics
	PROF_ParticleManager,
	PROF_ParticleEmitter,
	PROF_ParticleBoundary,
	PROF_CompParticleEmitter,

	PROF_ModuleEditor, // UI
	PROF_ThumbnailManager,

	PROF_ModuleRender, // Rendering
	PROF_FBOManager,
	PROF_GLCache,

	PROF_ModuleAudio, //Audio

	PROF_FileSystem, // IO
	PROF_ResourcesManager,
	PROF_ModelImporter,
	PROF_ShaderImporter,
	PROF_ECSImporter,
	PROF_TextureImporter,
	PROF_SkyboxImporter,
	PROF_InternalResources,

	PROF_CLASS_MAX
};

#ifdef PROFILING_ENABLED
#ifdef INTERNAL_PROFILING
#ifdef _DEBUG
#define PROFILING_OUTPUT_FILE_NAME "Debug Red Eye Profiling.json"
#else
#define PROFILING_OUTPUT_FILE_NAME "Release Red Eye Profiling.json"
#endif // _DEBUG

#include <EASTL/vector.h>

namespace RE_Profiler
{
	void Start();
	void Pause();
	void Clear();
	void Reset();
	void Deploy(const char* file_name = PROFILING_OUTPUT_FILE_NAME);
	void Exit();
};

struct ProfilingOperation
{
	RE_ProfiledFunc function;
	RE_ProfiledClass context;
	unsigned long long start; // ticks
	unsigned long long duration; // ticks
	unsigned long frame;
};

struct ProfilingTimer
{
	ProfilingTimer(RE_ProfiledFunc function, RE_ProfiledClass context);
	~ProfilingTimer();

	unsigned long long Read() const;
	float ReadMs() const;

	bool pushed;
	unsigned int operation_id;

	static unsigned long long start;
	static bool recording;
	static unsigned long frame;
	static eastl::vector<ProfilingOperation> operations;
};

#define RE_PROFILE(func, context) ProfilingTimer profiling_timer(func, context)
#define RE_PROFILE_FRAME() ProfilingTimer::frame++;

#else

namespace RE_Profiler
{
	char* GetFunctionStr(RE_ProfiledFunc function)
	{
		switch (function)
		{
		case PROF_Init: return "Init";
		case PROF_Start: return "Start";
		case PROF_PreUpdate: return "PreUpdate";
		case PROF_Update: return "Update";
		case PROF_PostUpdate: return "PostUpdate";
		case PROF_CleanUp: return "CleanUp";
		case PROF_Load: return "Load";
		case PROF_Save: return "Save";
		case PROF_Clear: return "Clear";

		case PROF_ReadAssetChanges: return "Read Asset Changes";
		case PROF_DroppedFile: return "Dropped File";

		case PROF_GetActiveShaders: return "Get Active Shaders";
		case PROF_DrawScene: return "DrawS cene";
		case PROF_DrawSkybox: return "Draw Skybox";
		case PROF_DrawStencil: return "Draw Stencil";
		case PROF_DrawEditor: return "Draw Editor";
		case PROF_DrawDebug: return "Draw Debug";
		case PROF_DrawThumbnails: return "Draw Thumbnails";
		case PROF_DrawParticles: return "Draw Particles";
		case PROF_DrawParticlesLight: return "Camera Particles' Light";

		case PROF_CameraRaycast: return "Camera Raycast";
		case PROF_EditorCamera: return "Editor Camera";

		case PROF_ThumbnailResources: return "Thumbnail Resources";
		case PROF_InitChecker: return "Init Checker";
		case PROF_InitShaders: return "Init Shaders";
		case PROF_InitWater: return "Init Water";
		case PROF_InitMaterial: return "Init Material";
		case PROF_InitSkyBox: return "Init SkyBox";

		case PROF_SetWindowProperties: return "Set Window Properties";
		case PROF_CreateWindow: return "Create Window";

		case PROF_ParticleTiming: return "Particle Timing";
		case PROF_ParticleUpdate: return "Particle Update";
		case PROF_ParticleSpawn: return "Particle Spawn";
		case PROF_ParticleCollision: return "Particle Collision";
		case PROF_ParticleBoundPCol: return "Particle Plane Boundary Collision";
		case PROF_ParticleBoundSCol: return "Particle Sphere Boundary Collision";

		default: return "Undefined";
		}
	}
}

#include <Optick/optick.h>

#define RE_OPTICK_CATEGORY(category)\
	(category < PROF_ModuleInput	?	Optick::Category::GameLogic :	\
	(category < PROF_ModuleScene	?	Optick::Category::Input :		\
	(category < PROF_ModulePhysics	?	Optick::Category::Scene :		\
	(category < PROF_ModuleEditor	?	Optick::Category::Physics :		\
	(category < PROF_ModuleRender	?	Optick::Category::UI :			\
	(category < PROF_ModuleAudio	?	Optick::Category::Rendering :	\
	(category < PROF_FileSystem 	?	Optick::Category::Audio : Optick::Category::IO)))))))

#define RE_PROFILE(func, context) OPTICK_CATEGORY(RE_Profiler::GetFunctionStr(func), RE_OPTICK_CATEGORY(context))
#define RE_PROFILE_FRAME() OPTICK_FRAME("MainThread RedEye")

#endif // INTERNAL_PROFILING

#else

#define RE_PROFILE(func, context)
#define RE_PROFILE_FRAME()

#endif // PROFILING_ENABLED

#endif // !__RE_PROFILER__