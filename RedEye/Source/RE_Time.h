#ifndef __TIMEMANAGER_H__
#define __TIMEMANAGER_H__

class Timer
{
public:
	Timer(const bool start_active = true);
	Timer(const Timer& timer);

	void Start();
	void Pause();
	void Stop();

	float ReadF() const;
	unsigned int Read() const;

private:

	bool paused = false;
	unsigned int started_at = 0, paused_at = 0;
};

enum GameState : char
{
	GS_TICK,
	GS_PLAY,
	GS_PAUSE,
	GS_STOP,
	GS_EMPTY
};

namespace RE_Time
{
	float FrameDeltaTime();
	unsigned int FrameExtraMS();

	GameState DrawEditorControls(); // Draws graphs on time stats
	void DrawEditorGraphs(); // Draws graphs on time stats
	
	void SetMaxFPS(float max_fps); // Set to 0 uncap fps
	void Delay(unsigned int ms);

	void TickGameTimer();
	void StartGameTimer();
	void PauseGameTimer();
	void StopGameTimer();

	float GetDeltaTime();
	float GetMaxFPS();

	float GetEngineTimer();
	float GetGameTimer();
	float GetCurrentTimer();

	GameState GetState();

	unsigned int GetCappedMS();
	unsigned int GetFpsCounter();
	unsigned int GetLastMs();
	unsigned int GetLastFPS();

	namespace Internal
	{
		static GameState state = GS_STOP;
		static Timer
			ms_timer(false), // read every frame
			fps_timer(false), // read every second
			game_timer(false); // read when playing scene

		static float
			dt = 0,
			capped_fps = 60.f;

		static unsigned int
			capped_ms = 16u,
			last_fps_count = 0u, last_ms_count = 0u,
			fps_counter = 0u;

		static unsigned long global_frame_counter = 0u;

		static bool	pause_plotting = false;
		static float fps[100] = {}, ms[100] = {};
	}
};

#endif // !__TIMEMANAGER_H__