#ifndef __RE_PROFILER__
#define __RE_PROFILER__

//#define PROFILING_ENABLED // undefine to disable any profiling methods
#define INTERNAL_PROFILING // undefine to use Optick Profiling
#define RECORD_FROM_START true

// define to set engine to run particle physics or rendering demo test
//#define PARTICLE_PHYSICS_TEST
//#define PARTICLE_RENDER_TEST

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

#ifdef PARTICLE_PHYSICS_TEST
#undef PARTICLE_RENDER_TEST
#ifdef _DEBUG
#define PROFILING_OUTPUT_FILE_NAME "Debug Physics Red Eye Profiling.json"
#else
#define PROFILING_OUTPUT_FILE_NAME "Release Physics Red Eye Profiling.json"
#endif // _DEBUG

#elif defined(PARTICLE_RENDER_TEST)
#undef PARTICLE_PHYSICS_TEST
#ifdef _DEBUG
#define PROFILING_OUTPUT_FILE_NAME "Debug Rendering Red Eye Profiling.json"
#else
#define PROFILING_OUTPUT_FILE_NAME "Release Rendering Red Eye Profiling.json"
#endif // _DEBUG

#else
#ifdef _DEBUG
#define PROFILING_OUTPUT_FILE_NAME "Debug General Red Eye Profiling.json"
#else
#define PROFILING_OUTPUT_FILE_NAME "Release General Red Eye Profiling.json"
#endif // _DEBUG
#endif // PARTICLE_PHYSICS_TEST

#include <EASTL/vector.h>

struct ProfilingOperation
{
	RE_ProfiledFunc function;
	RE_ProfiledClass context;
	unsigned long long start; // ticks
	unsigned long long duration; // ticks
	unsigned long frame;

#if defined(PARTICLE_PHYSICS_TEST) || defined(PARTICLE_RENDER_TEST)

	unsigned int p_count = 0u;

#ifdef PARTICLE_PHYSICS_TEST

	unsigned int p_col_internal = 0u;
	unsigned int p_col_boundary = 0u;

#elif defined(PARTICLE_RENDER_TEST)

	unsigned int p_lights = 0u;

#endif // PARTICLE_RENDER_TEST
#endif // PARTICLE_PHYSICS_TEST || PARTICLE_RENDER_TEST
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

#if defined(PARTICLE_PHYSICS_TEST) || defined(PARTICLE_RENDER_TEST)

	static int wait4frame;
	static int current_sim;
	static unsigned int update_time;
	static unsigned int p_count;

#ifdef PARTICLE_PHYSICS_TEST

	static unsigned int p_col_internal;
	static unsigned int p_col_boundary;

#else

	static unsigned int p_lights;

#endif // PARTICLE_PHYSICS_TEST
#endif // PARTICLE_PHYSICS_TEST || PARTICLE_RENDER_TEST
};

namespace RE_Profiler
{
	void Start();
	void Pause();
	void Clear();
	void Reset();
	void Deploy(const char* file_name = PROFILING_OUTPUT_FILE_NAME);
	void Exit();
};

#define RE_PROFILE(func, context) ProfilingTimer profiling_timer(func, context)
#define RE_PROFILE_FRAME() ProfilingTimer::frame++;

#else

#include <Optick/optick.h>
#define RE_OPTICK_NAME(function)\ // TODO RUB: missing some definitions & should be improved
	function == PROF_Init ? "Init" : \
	function == PROF_Start ? "Start" : \
	function == PROF_PreUpdate ? "PreUpdate" : \
	function == PROF_Update ? "Update" : \
	function == PROF_PostUpdate ? "PostUpdate" : \
	function == PROF_CleanUp ? "CleanUp" : \
	function == PROF_Load ? "Load" : \
	function == PROF_Save ? "Save" : \
	function == PROF_Clear ? "Clear" : \
	function == PROF_ReadAssetChanges ? "Read Asset Changes" : \
	function == PROF_DroppedFile ? "Dropped File" : \
	function == PROF_DrawScene ? "Draw Scene" : \
	function == PROF_DrawSkybox ? "Draw Skybox" : \
	function == PROF_DrawStencil ? "Draw Stencil" : \
	function == PROF_DrawEditor ? "Draw Editor" : \
	function == PROF_DrawDebug ? "Draw Debug" : \
	function == PROF_CameraRaycast ? "Camera Raycast" : \
	function == PROF_EditorCamera ? "Editor Camera" : "Undefined"

#define RE_OPTICK_CATEGORY(category)\
	(category < PROF_ModuleInput	?	Optick::Category::GameLogic :	\
	(category < PROF_ModuleScene	?	Optick::Category::Input :		\
	(category < PROF_ModulePhysics	?	Optick::Category::Scene :		\
	(category < PROF_ModuleEditor	?	Optick::Category::Physics :		\
	(category < PROF_ModuleRender	?	Optick::Category::UI :			\
	(category < PROF_ModuleAudio	?	Optick::Category::Rendering :	\
	(category < PROF_FileSystem 	?	Optick::Category::Audio : Optick::Category::IO)))))))

#define RE_PROFILE(func, context) OPTICK_CATEGORY(RE_OPTICK_NAME(func),	RE_OPTICK_CATEGORY(context))
#define RE_PROFILE_FRAME() OPTICK_FRAME("MainThread RedEye")

#undef PARTICLE_PHYSICS_TEST
#undef PARTICLE_RENDER_TEST

#endif // INTERNAL_PROFILING

#else

#define RE_PROFILE(func, context)
#define RE_PROFILE_FRAME()

#undef INTERNAL_PROFILING
#undef PARTICLE_PHYSICS_TEST
#undef PARTICLE_RENDER_TEST

#endif // PROFILING_ENABLED

#endif // !__RE_PROFILER__