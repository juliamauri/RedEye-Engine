#ifndef _RE_TIMER_H__
#define _RE_TIMER_H__

class RE_Timer
{
public:
	RE_Timer(const bool start_active = true)
	{ start_active ? Start() : Stop(); }

	RE_Timer(const RE_Timer& timer) :
		paused(timer.paused),
		started_at(timer.started_at),
		paused_at(timer.paused_at) {}

	~RE_Timer() {}

	void Start();
	void Pause();

	void Stop()
	{
		paused = true;
		started_at = paused_at = 0u;
	}

	unsigned int Read() const;
	float ReadF() const { return static_cast<float>(Read()); }
	
private:

	bool paused = false;
	unsigned int started_at = 0, paused_at = 0;
};

#endif // !_RE_TIMER_H__