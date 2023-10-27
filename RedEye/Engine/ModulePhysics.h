#ifndef __MODULEPHYSICS__
#define __MODULEPHYSICS__

namespace ModulePhysics
{
	enum class UpdateMode : int
	{
		ENGINE_PAR,
		FIXED_UPDATE,
		FIXED_TIME_STEP
	};

	namespace
	{
		float fixed_dt = 1.f / 30.f;
		float dt_offset = 0.f;

		float update_count = 0.f;
		float time_counter = 0.f;
		float updates_per_s = 0.f;

		UpdateMode mode = UpdateMode::FIXED_UPDATE;
	}

	void Update();
	void CleanUp();

	void OnPlay(bool was_paused);
	void OnPause();
	void OnStop();

	void DrawDebug();
	void DrawEditor();

	void Load();
	void Save();
};

#endif // !__MODULEPHYSICS__