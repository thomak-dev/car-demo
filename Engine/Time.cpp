#include "Time.h"
#include <SDL.h>
#include <algorithm>

Time::Time(int maxFrameRate, int fixedRate)
	:minFrameDelta{ static_cast<int>(ceil(1000.f / maxFrameRate)) },
	internalOldTime{ SDL_GetTicks() },
	newTime{ internalOldTime },
	fixedTimeStep{ static_cast<int>(floor(1000.f / fixedRate)) }
{
	if (minFrameDelta <= 0)
		minFrameDelta = 1;
	if (fixedTimeStep < minFrameDelta)
		fixedTimeStep = minFrameDelta;

	smoothFrames = ceil(GetMaxFrameRate() / 6);
}

void Time::BusyTick(bool& variable, bool& fixed)
{
	newTime = SDL_GetTicks();
	internalDtMs = newTime - internalOldTime;
	
	int ahead = std::max(0, minFrameDelta - internalDtMs);
	if (ahead - delayedTime <= 0)
	{
		++frameCount;
		variable = true;
		delayedTime = ahead - delayedTime;
		cappedDtMs = newTime - oldLimitedTime;

		smoothTime += cappedDtMs;
		if (frameCount % smoothFrames == 0)
		{
			smoothFps = 1000 / (static_cast<float>(smoothTime) / smoothFrames);
			smoothTime = 0;
		}
		oldLimitedTime = newTime;
	}
	else
	{
		variable = false;
		delayedTime += internalDtMs;
	}

	accumFixedStep += internalDtMs;
	if (accumFixedStep >= fixedTimeStep)
	{
		accumFixedStep -= fixedTimeStep;
		fixed = true;
	}
	else
		fixed = false;

	internalOldTime = newTime;
}

void Time::EcoTick(bool& variable, bool& fixed)
{
	
	newTime = SDL_GetTicks();
	internalDtMs = newTime - internalOldTime;

	int aheadOfCap = std::max(0, minFrameDelta - (internalDtMs + variableOvershoot));
	int aheadOfFixed = std::max(0, fixedTimeStep - (internalDtMs + accumFixedStep));

	int delay = std::min(aheadOfCap, aheadOfFixed);
	if(delay)
	{
		SDL_Delay(delay);
		newTime = SDL_GetTicks();
		internalDtMs = newTime - internalOldTime;
	}

	variableOvershoot = internalDtMs - aheadOfCap;

	if (variableOvershoot < 0)
	{
		variableOvershoot = 0;
		variable = false;
	}
	else
	{
		++frameCount;
		variable = true;
		cappedDtMs = newTime - oldLimitedTime;

		smoothTime += cappedDtMs;
		if (frameCount % smoothFrames == 0)
		{
			smoothFps = 1000 / (static_cast<float>(smoothTime) / smoothFrames);
			smoothTime = 0;
		}

		oldLimitedTime = newTime;
	}

	accumFixedStep += internalDtMs;
	if (accumFixedStep >= fixedTimeStep)
	{
		accumFixedStep -= fixedTimeStep;
		fixed = true;
	}
	else
		fixed = false;

	internalOldTime = newTime;
}