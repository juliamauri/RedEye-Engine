#ifndef __TIMEMANAGER_H__
#define __TIMEMANAGER_H__

#include <list>

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

class TimeManager
{
public:

	TimeManager(float max_fps = 60.f);
	~TimeManager();

	float	UpdateDeltaTime(); // Called before updating modules
	void	ManageFrameTimers(); // Called after modules update
	void	DrawEditor(); // Draws graphs on time stats
	
	void	SetMaxFPS(float max_fps); // Set to 0 uncap fps
	float	GetMaxFPS() const;
	float	GetDeltaTime() const;
	unsigned int GetCappedMS() const;
	unsigned int GetFpsCounter() const;
	unsigned int GetLastMs() const;
	unsigned int GetLastFPS() const;

private:

	void ClearArrays();

private:

	unsigned long	frames_counter;
	unsigned int	fps_counter;
	unsigned int	last_fps_count;
	unsigned int	last_ms_count;

	float	dt;
	float	capped_fps;
	unsigned int	capped_ms;

	Timer	ms_timer; // read every frame
	Timer	fps_timer; // read every second

	float	fps[100] = {};
	float	ms[100] = {};
	bool	pause_plotting;
};

#endif // !__TIMEMANAGER_H__