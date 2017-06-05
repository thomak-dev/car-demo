#include "Time.h"
#include <SDL.h>
#include <glm\glm.hpp>

Time::Time(int maxFrameRate, int fixedRate)
	:maxFrameRate{ maxFrameRate },
	smoothFrames{ 15 },
	minFrameDelta{ static_cast<uint32_t>(glm::ceil(1000.f / maxFrameRate)) },
	oldTime{ SDL_GetTicks() },
	newTime{ oldTime },
	fixedTimeStep{ static_cast<uint32_t>(glm::ceil(1000.f / fixedRate)) }
{
	SDL_assert(fixedRate <= maxFrameRate);
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
	if (dtMs < minFrameDelta)
	{
		SDL_Delay(minFrameDelta - dtMs);
		newTime = SDL_GetTicks();
		dtMs = newTime - oldTime;
	}
	smoothTime += dtMs;
	if (frameCount % smoothFrames == 0)
	{
		smoothFps = 1000 / (static_cast<float>(smoothTime) / smoothFrames);
		smoothTime = 0;
	}
	accumulated += dtMs;
	oldTime = newTime;
	if(accumulated >= fixedTimeStep)
	{
		accumulated -= fixedTimeStep;
		return true;
	}
	return false;
}