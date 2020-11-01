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

	bool paused;
	unsigned int started_at;
	unsigned int paused_at;
};

struct TimePlotting
{
	TimePlotting();

	void DrawEditor();
	unsigned int ManageFrameTimers(unsigned int ms_count);
	void SetMaxFPS(float max_fps);
	inline void ClearArrays();

	Timer	fps_timer; // read every second

	unsigned long	frames_counter = 0u;
	unsigned int	fps_counter = 0u;
	unsigned int	last_fps_count = 0u;
	unsigned int	last_ms_count = 0u;

	float			capped_fps = 60.f;
	unsigned int	capped_ms = 0u;

	bool	pause_plotting = false;
	float	fps[100] = {};
	float	ms[100] = {};
};

class RE_TimeManager
{
public:

	RE_TimeManager();
	~RE_TimeManager();

	void Init(float max_fps);

	float UpdateDeltaTime(); // returns updated dt
	unsigned int ManageFrameTimers(); // returns extra ms for frame

	void	DrawEditor(); // Draws graphs on time stats
	
	void	SetMaxFPS(float max_fps); // Set to 0 uncap fps
	float	GetMaxFPS() const;

	unsigned int GetCappedMS() const;
	unsigned int GetFpsCounter() const;
	unsigned int GetLastMs() const;
	unsigned int GetLastFPS() const;

	static void	Delay(unsigned int ms);

	static float GetDeltaTime();
	static float GetEngineTimer();
	static float GetGameTimer();

	void StartGameTimer();
	void PauseGameTimer();
	void StopGameTimer();

private:

	static float dt;
	static Timer	engine_timer;
	static Timer	game_timer;

	Timer	ms_timer; // read every frame

	TimePlotting* plot_data = nullptr;
};

#endif // !__TIMEMANAGER_H__