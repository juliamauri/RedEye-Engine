#pragma once

#define PROFILE_ACTIVE
#define INTERNAL_PROFILING
#define PROFILE_CAP 9400

enum RE_ProfiledFunc : unsigned short
{
	PROF_Init,
	PROF_Start,
	PROF_PreUpdate,
	PROF_Update,
	PROF_PostUpdate,
	PROF_CleanUp,
	PROF_Load,
	PROF_Save,
	PROF_Clear,

	PROF_ReadAssetChanges,
	PROF_DroppedFile,

	PROF_GetActiveShaders,
	PROF_DrawScene,
	PROF_DrawSkybox,
	PROF_DrawStencil,
	PROF_DrawEditor,
	PROF_DrawDebug,
	PROF_DrawThumbnails,

	PROF_CameraRaycast,
	PROF_EditorCamera,

	PROF_ThumbnailResources,
	PROF_InitChecker,
	PROF_InitShaders,
	PROF_InitWater,
	PROF_InitMaterial,
	PROF_InitSkyBox,

	PROF_SetWindowProperties,
	PROF_CreateWindow,

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

#ifdef PROFILE_ACTIVE
#ifdef INTERNAL_PROFILING

#include <EASTL\vector.h>

struct ProfilingTimer
{
	ProfilingTimer(RE_ProfiledFunc function, RE_ProfiledClass context);
	~ProfilingTimer();
	unsigned int operation_id;
	static unsigned int frame;
};

struct ProfilingOperation
{
	RE_ProfiledFunc function;
	RE_ProfiledClass context;
	unsigned long long start; // ticks
	unsigned long long duration; // ticks
	unsigned int frame;
};

struct RE_Profiler
{
	RE_Profiler();
	~RE_Profiler();
	eastl::vector<ProfilingOperation> operations;

#ifdef PROFILE_CAP
	void Frame();
#endif // PROFILE_CAP
};

#define RE_PROFILE(func, context) ProfilingTimer profiling_timer(func, context)

#ifdef PROFILE_CAP
#define RE_PROFILE_FRAME() ++ProfilingTimer::frame; profiler->Frame()
#else
#define RE_PROFILE_FRAME() ++ProfilingTimer::frame
#endif // PROFILE_CAP


#else
struct RE_Profiler {};
#include "Optick\include\optick.h"

#define RE_OPTICK_NAME(function)\
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
	(category < PROF_ModuleInput ?	Optick::Category::GameLogic :	\
	(category < PROF_ModuleScene ?	Optick::Category::Input :		\
	(category < PROF_ModuleEditor ?	Optick::Category::Scene :		\
	(category < PROF_ModuleRender ?	Optick::Category::UI :			\
	(category < PROF_ModuleAudio ?	Optick::Category::Rendering :	\
	(category < PROF_FileSystem ?	Optick::Category::Audio :		\
									Optick::Category::IO))))))

#define RE_PROFILE(func, context) OPTICK_CATEGORY(RE_OPTICK_NAME(func),	RE_OPTICK_CATEGORY(context))
#define RE_PROFILE_FRAME() OPTICK_FRAME("MainThread RedEye")
#endif // INTERNAL_PROFILING
#else
struct RE_Profiler {};
#define RE_PROFILE(func, context)
#define RE_PROFILE_FRAME()
#endif // PROFILE_ACTIVE
