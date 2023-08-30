#ifndef __RE_PROFILER__
#define __RE_PROFILER__

//#define PROFILING_ENABLED // undefine to disable any profiling 

#ifdef PROFILING_ENABLED
#define INTERNAL_PROFILING // undefine to use Optick Profiling
#endif // PROFILING_ENABLED

#ifdef INTERNAL_PROFILING
#include "RE_DataTypes.h"
#include <MGL/Time/Clock.h>
#include <EASTL/vector.h>
#endif

enum class RE_ProfiledFunc : unsigned short
{
	Init, // Modules
	Start,
	PreUpdate,
	Update,
	PostUpdate,
	CleanUp,
	Load,
	Save,
	Clear,

	ReadAssetChanges, // File System
	DroppedFile,

	GetActiveShaders, // Rendering
	DrawScene,
	DrawSkybox,
	DrawStencil,
	DrawEditor,
	DrawDebug,
	DrawThumbnails,
	DrawParticles,
	DrawParticlesLight,

	CameraRaycast, // Cameras
	EditorCamera,

	ThumbnailResources, // Resources
	InitChecker,
	InitShaders,
	InitWater,
	InitMaterial,
	InitSkyBox,

	SetWindowProperties, // Window
	CreateWindow,

	ParticleTiming, // Particles
	ParticleUpdate,
	ParticleSpawn,
	ParticleCollision,
	ParticleBoundPCol,
	ParticleBoundSCol,

	CheckHardware, // Utility

	END
};

enum class RE_ProfiledClass : unsigned short
{
	Application, // GameLogic
	Log,
	Time,
	Math,
	Hardware,

	ModuleInput, // Input
	ModuleWindow,

	ModuleScene, // Scene
	CameraManager,
	PrimitiveManager,

	ModulePhysics, // Physics
	ParticleManager,
	ParticleEmitter,
	ParticleBoundary,
	CompParticleEmitter,

	ModuleEditor, // UI
	ThumbnailManager,

	ModuleRender, // Rendering
	RenderView,
	FBOManager,
	GLCache,

	ModuleAudio, //Audio

	FileSystem, // IO
	ResourcesManager,
	ModelImporter,
	ShaderImporter,
	ECSImporter,
	TextureImporter,
	SkyboxImporter,
	InternalResources,

	END
};

#ifdef PROFILING_ENABLED

namespace RE_Profiler
{
	const char* GetFunctionStr(const RE_ProfiledFunc function);
}

#ifdef INTERNAL_PROFILING

namespace RE_Profiler
{
	constexpr bool RecordFromStart = true;

	const char* GetClassStr(const RE_ProfiledClass function);

	void Start();
	void Pause();
	void Clear();
	void Reset();
	void Exit();
	void Deploy();
};

struct ProfilingOperation
{
	ProfilingOperation() = default;
	ProfilingOperation(
		const RE_ProfiledFunc& function,
		const RE_ProfiledClass& context,
		const math::tick_t& start,
		const math::tick_t& duration,
		const ulong& frame);

	bool operator==(const ProfilingOperation& other) const;

	RE_ProfiledFunc function;
	RE_ProfiledClass context;
	math::tick_t start; // ticks
	math::tick_t duration; // ticks
	ulong frame;
};

struct ProfilingTimer
{
	static math::tick_t start;
	static bool recording;
	static ulong frame;
	static eastl::vector<ProfilingOperation> operations;

	ProfilingTimer() = default;
	ProfilingTimer(bool pushed, const eastl_size_t& operation_id);
	ProfilingTimer(RE_ProfiledFunc function, RE_ProfiledClass context);
	~ProfilingTimer();

	bool operator==(const ProfilingTimer& other) const;

	math::tick_t Read() const;
	float ReadMs() const;

	bool pushed;
	eastl_size_t operation_id = 0;
};

#define RE_PROFILE(func, context) ProfilingTimer profiling_timer(func, context)
#define RE_PROFILE_FRAME() ProfilingTimer::frame++;

#else

#include <Optick/optick.h>

#define RE_OPTICK_CATEGORY(category)\
	(category < RE_ProfiledClass::ModuleInput	?	Optick::Category::GameLogic :	\
	(category < RE_ProfiledClass::ModuleScene	?	Optick::Category::Input :		\
	(category < RE_ProfiledClass::ModulePhysics	?	Optick::Category::Scene :		\
	(category < RE_ProfiledClass::ModuleEditor	?	Optick::Category::Physics :		\
	(category < RE_ProfiledClass::ModuleRender	?	Optick::Category::UI :			\
	(category < RE_ProfiledClass::ModuleAudio	?	Optick::Category::Rendering :	\
	(category < RE_ProfiledClass::FileSystem 	?	Optick::Category::Audio : Optick::Category::IO)))))))

#define RE_PROFILE(func, context) OPTICK_CATEGORY(RE_Profiler::GetFunctionStr(func), RE_OPTICK_CATEGORY(context))
#define RE_PROFILE_FRAME() OPTICK_FRAME("MainThread RedEye")

#endif // INTERNAL_PROFILING

#else

#define RE_PROFILE(func, context)
#define RE_PROFILE_FRAME()

#endif // PROFILING_ENABLED

#endif // !__RE_PROFILER__