#ifndef __RE_PROFILER__
#define __RE_PROFILER__

#define PROFILING_ENABLED // undefine to disable any profiling 
#ifdef PROFILING_ENABLED
#define INTERNAL_PROFILING // undefine to use Optick Profiling
#endif // PROFILING_ENABLED

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

#include <EASTL/vector.h>

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
	void WriteToFile(const char* filename, const char* buffer);
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