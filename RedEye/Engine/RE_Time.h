#ifndef _RE_TIME_H__
#define _RE_TIME_H__

#include "RE_Timer.h"

enum GameState : char
{
	GS_TICK,
	GS_PLAY,
	GS_PAUSE,
	GS_STOP,
	GS_EMPTY
};

class RE_Time
{
public:
	RE_Time();
	~RE_Time() {}

	float FrameDeltaTime()
	{
		dt = ms_timer.ReadF() / 1000.f;
		ms_timer.Start();
		return dt;
	}

	unsigned int FrameExtraMS()
	{
		++global_frame_counter;
		++fps_counter;

		if (fps_timer.Read() >= 1000)
		{
			last_fps_count = fps_counter;
			fps_counter = 0u;
			fps_timer.Start();
		}

		last_ms_count = ms_timer.Read();

		unsigned int ret = 1u;
		if (capped_ms > 0 && capped_ms > last_ms_count) ret = capped_ms - last_ms_count;

		return ret;
	}

	GameState DrawEditorControls(); // Draws graphs on time stats
	void DrawEditorGraphs(); // Draws graphs on time stats

	void Delay(unsigned int ms) const;
	
	void SetMaxFPS(float max_fps) // Set 0 to uncap fps
	{
		capped_fps = max_fps;
		if (capped_fps < 1.f) capped_ms = 0u;
		else capped_ms = static_cast<unsigned int>(1000.f / capped_fps);
	}

	float GetMaxFPS() const { return capped_fps; }
	float GetDeltaTime() const { return dt; }

	unsigned int GetCappedMS() const { return capped_ms; }
	unsigned int GetFpsCounter() const { return fps_counter; }
	unsigned int GetLastMs() const { return last_ms_count; }
	unsigned int GetLastFPS() const { return last_fps_count; }

	GameState GetState() const { return state; }

	unsigned int GetTicks() const;
	float GetEngineTimer() const;
	float GetGameTimer() const { return game_timer.ReadF() / 1000.f; }
	float GetCurrentTimer() const { return (state == GS_STOP) ? GetEngineTimer() : game_timer.ReadF() / 1000.f; }

	void TickGameTimer() { game_timer.Start(); state = GS_TICK; }
	void StartGameTimer() { game_timer.Start(); state = GS_PLAY; }
	void PauseGameTimer() { game_timer.Pause(); state = GS_PAUSE; }
	void StopGameTimer() { game_timer.Stop(); state = GS_STOP; }

private:

	// Frame time data
	float dt = 0, capped_fps = 60.f;
	unsigned int capped_ms = 16u, last_fps_count = 0u, last_ms_count = 0u, fps_counter = 0u;
	unsigned long global_frame_counter = 0u;

	// Timers
	GameState state = GS_STOP;
	RE_Timer
		ms_timer, // read every frame
		fps_timer, // read every second
		game_timer; // read when playing scene

	// Profiling
	bool pause_plotting = false;
	float fps[100] = {}, max_fps = 0.f, ms[100] = {};
};

#endif // !_RE_TIME_H__