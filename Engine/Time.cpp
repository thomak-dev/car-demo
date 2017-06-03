#include "Time.h"
#include <SDL.h>
#include <glm\glm.hpp>

Time::Time(int maxFrameRate)
	:maxFrameRate{ maxFrameRate },
	minFrameDelta{ static_cast<uint32_t>(glm::ceil(1000.f / maxFrameRate)) },
	oldTime{ SDL_GetTicks() },
	newTime{ oldTime },
	smoothFrames{ 15 }
{
}


void Time::BeginNewFrame()
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
	oldTime = newTime;
}

float Time::GetFps() const
{
	return smoothFps;
}

float Time::GetDeltaTime() const
{
	return dtMs / 1000.f;
}

float Time::GetTime() const
{
	return 1000.f/newTime;
}

uint32_t Time::GetTimeMs() const
{
	return newTime;
}


int Time::GetFrameCount() const
{
	return frameCount;
}