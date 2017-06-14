#pragma once
#include <cstdint>
#include <chrono>
#include "Singleton.h"

/**
 * \brief 
 * Simple helper class to deal with time and framerate in main loops
 * Supports fixed and variable time step, as well as framerate capping
 */
class Time : public Singleton<Time>
{
public:
	using Clock = std::chrono::high_resolution_clock;
	using Duration = Clock::duration;
	using TimePoint = Clock::time_point;
	using Period = Clock::period;
	explicit Time(double maxRate = 120, double fixedRate = 60);

	/**
	 * \brief 
	 * Iterate time
	 * \param variable
	 * Set to true when it's time for a variable, capped time step, to false otherwise
	 * \param fixed
	 * Set to true when it's time for a fixed time step, to false otherwise
	 */
	void BusyTick(bool& variable, bool& fixed);
	/**
	 * \brief 
	 * Iterate time economically, meaning the thread will be put to sleep until the next time step.
	 * This is less accurate and smooth than BusyTick because of OS scheduling.
	 * \param variable
	 * Set to true when it's time for a variable, capped time step, to false otherwise
	 * \param fixed
	 * Set to true when it's time for a fixed time step, to false otherwise
	 */
	void EcoTick(bool& variable, bool& fixed);

	float GetSmoothFps() const { return smoothFps; }
	void SetSmoothCount(int count) { smoothFrames = count; }
	int GetSmoothCount() const { return smoothFrames; }
	float GetDeltaTime() const { return cappedDt.count() / SecondsFactor; }
	float GetDeltaTimeMs() const { return GetDeltaTime() / 1000; }
	float GetTime() const { return SecondsFactor / (oldInternalTime - startTime).count(); }
	float GetTimeMs() const { return GetTime() / 1000; }
	uint64_t GetCappedFrameCount() const { return cappedFrameCount; }
	uint64_t GetFixedFrameCount() const { return fixedFrameCount; }
	float GetFixedTimeStep() const { return fixedTimeStep.count() / SecondsFactor; }
	float GetFixedTimeStepMs() const { return GetFixedTimeStep() / 1000; }
	float GetMaxRate() const { return SecondsFactor / minTimeStep.count(); }
	float GetFixedRate() const { return SecondsFactor / fixedTimeStep.count(); }

private:
	static constexpr float SecondsFactor{ static_cast<float>(Period::den) };
	Clock clock;
	const TimePoint startTime;
	Duration minTimeStep{};
	Duration fixedTimeStep{};
	
	TimePoint oldInternalTime{};
	TimePoint oldCappedTime{};

	Duration smoothTime{};
	Duration internalDt{};
	Duration cappedDt{};
	Duration accumFixedStep{};
	
	float smoothFps{};
	int smoothFrames{};

	uint64_t internalFrameCount{};
	uint64_t cappedFrameCount{};
	uint64_t fixedFrameCount{};

	Duration delayedTime{};
	Duration variableOvershoot{};
	
};

