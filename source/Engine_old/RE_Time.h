#ifndef _RE_TIME_H__
#define _RE_TIME_H__

#include "RE_Timer.h"
#include "RE_DataTypes.h"

#include <EASTL/array.h>

namespace RE_Time
{
	// Time Controls
	float FrameDeltaTime();
	uint FrameExtraMS();

	void Delay(uint ms);
	void SetMaxFPS(float max_fps); // Set 0 to uncap fps

	// Getters
	float MaxFPS();
	float DeltaTime();

	uint CappedMS();
	uint FpsCounter();
	uint LastMs();
	uint LastFPS();
	uint Ticks();

	float EngineTimer();
	float GameTimer();
	float CurrentTimer();

	// Game Timer Controls
	void TickGameTimer();
	void StartGameTimer();
	void PauseGameTimer();
	void StopGameTimer();

	// Game State
	enum class GameState : uchar
	{
		TICK,
		PLAY,
		PAUSE,
		STOP,
		NONE
	};

	GameState State();
	bool StateIs(GameState state);

	// Editor Draws
	GameState DrawEditorControls(); // Draws graphs on time stats
	void DrawEditorGraphs(); // Draws graphs on time stats
};

#endif // !_RE_TIME_H__