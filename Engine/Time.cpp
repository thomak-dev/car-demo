#include "Time.h"
#include <algorithm>
#include <thread>

Time::Time(double maxRate, double fixedRate)
	:
	startTime{ clock.now() },
	minTimeStep( static_cast<Duration::rep>(Period::den / maxRate) ),
	fixedTimeStep{ static_cast<Duration::rep>(Period::den / fixedRate) },
	oldInternalTime{ startTime },
	oldCappedTime{ startTime }
{
	if (minTimeStep.count() <= 0)
		minTimeStep = Duration{ 1 };
	if (fixedTimeStep < minTimeStep)
		fixedTimeStep = minTimeStep;

	smoothFrames = static_cast<int>(ceil(GetMaxRate() / 6));
}

void Time::BusyTick(bool& variable, bool& fixed)
{
	++internalFrameCount;
	TimePoint newTime = clock.now();
	internalDt = newTime - oldInternalTime;
	
	Duration ahead = std::max(Duration::zero(), minTimeStep - internalDt);
	if (ahead - delayedTime <= Duration::zero())
	{
		++cappedFrameCount;
		variable = true;
		delayedTime = ahead - delayedTime;
		cappedDt = newTime - oldCappedTime;

		smoothTime += cappedDt;
		if (cappedFrameCount % smoothFrames == 0)
		{
			smoothFps = SecondsFactor / (static_cast<float>(smoothTime.count()) / smoothFrames);
			smoothTime = Duration::zero();
		}
		oldCappedTime = newTime;
	}
	else
	{
		variable = false;
		delayedTime += internalDt;
	}

	accumFixedStep += internalDt;
	if (accumFixedStep >= fixedTimeStep)
	{
		++fixedFrameCount;
		accumFixedStep -= fixedTimeStep;
		fixed = true;
	}
	else
		fixed = false;

	oldInternalTime = newTime;
}

void Time::EcoTick(bool& variable, bool& fixed)
{
	++internalFrameCount;
	TimePoint newTime = clock.now();
	internalDt = newTime - oldInternalTime;

	Duration aheadOfCap = std::max(Duration::zero(), minTimeStep - (internalDt + variableOvershoot));
	Duration aheadOfFixed = std::max(Duration::zero(), fixedTimeStep - (internalDt + accumFixedStep));

	Duration delay = std::min(aheadOfCap, aheadOfFixed);
	if(delay.count() > 0)
	{
		std::this_thread::sleep_for(delay);
		newTime = clock.now();
		internalDt = newTime - oldInternalTime;
	}

	if (aheadOfCap <= aheadOfFixed)
	{
		++cappedFrameCount;
		variable = true;
		cappedDt = newTime - oldCappedTime;

		smoothTime += cappedDt;
		if (cappedFrameCount % smoothFrames == 0)
		{
			smoothFps = SecondsFactor / (static_cast<float>(smoothTime.count()) / smoothFrames);
			smoothTime = Duration::zero();
		}

		variableOvershoot = Duration::zero();
		oldCappedTime = newTime;
	}
	else
	{
		variableOvershoot += internalDt;
		variable = false;
	}


	accumFixedStep += internalDt;
	if (accumFixedStep >= fixedTimeStep)
	{
		++fixedFrameCount;
		accumFixedStep -= fixedTimeStep;
		fixed = true;
	}
	else
		fixed = false;

	oldInternalTime = newTime;
}