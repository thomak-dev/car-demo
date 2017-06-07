#include "Time.h"
#include <SDL.h>
#include <glm\glm.hpp>
#include <algorithm>

Time::Time(int maxFrameRate, int fixedRate)
	:maxFrameRate{ maxFrameRate },
	smoothFrames{ maxFrameRate / 6 },
	minFrameDelta{ static_cast<int>(glm::ceil(1000.f / maxFrameRate)) },
	oldTime{ SDL_GetTicks() },
	newTime{ oldTime },
	fixedTimeStep{ static_cast<int>(glm::ceil(1000.f / fixedRate)) }
{
	SDL_assert(fixedRate > 0 && fixedRate <= maxFrameRate);
}

/**
 * \brief 
 * Process time measurement in main loop. Call this once per frame (loop iteration), preferably at the beginning.
 * 
 * \return 
 * true when a fixed time step has been completed, false otherwise.
 */
bool Time::BeginNewFrame()
{
	++frameCount;
	newTime = SDL_GetTicks();
	dtMs = newTime - oldTime;
	int delay = std::max(0, minFrameDelta - dtMs);
		
	if (delay > 0)
	{
		// Only delay when no fixed time step would be delayed
		int timeUntilFixed = fixedTimeStep - accumFixedStep;
		if(delay < timeUntilFixed)
		{
			SDL_Delay(delay + std::min(accumDelay, timeUntilFixed - delay));
			accumDelay = 0;
			newTime = SDL_GetTicks();
			dtMs = newTime - oldTime;
		}
		else if (strictLimiting)
			accumDelay += delay;
	}
	
	smoothTime += dtMs;
	if (frameCount % smoothFrames == 0)
	{
		smoothFps = 1000 / (static_cast<float>(smoothTime) / smoothFrames);
		smoothTime = 0;
	}
	accumFixedStep += dtMs;
	oldTime = newTime;
	if(accumFixedStep >= fixedTimeStep)
	{
		accumFixedStep -= fixedTimeStep;
		return true;
	}
	return false;
}