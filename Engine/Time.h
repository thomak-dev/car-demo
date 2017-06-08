#pragma once
#include <cstdint>
#include "Singleton.h"

/**
 * \brief 
 * Simple helper class to deal with time and framerate in main loops
 * Supports fixed and variable time step, as well as framerate capping
 */
class Time : public Singleton<Time>
{
public:
	explicit Time(int maxFrameRate = 144, int fixedRate = 60);
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
	 * \param fixed
	 * Set to true when it's time for a fixed time step, to false otherwise
	 */
	void EcoTick(bool& variable, bool& fixed);
	float GetSmoothFps() const { return smoothFps; }
	void SetSmoothCount(int count) { smoothFrames = count; }
	int GetSmoothCount() const { return smoothFrames; }
	float GetDeltaTime() const { return cappedDtMs / 1000.f; }
	int GetDeltaTimeMs() const { return cappedDtMs; }
	float GetTime() const { return 1000.f / newTime; }
	uint32_t GetTimeMs() const { return newTime; }
	uint32_t GetFrameCount() const { return frameCount; }
	float GetFixedTimeStep() const { return fixedTimeStep / 1000.0f; }
	int GetFixedTimeStepMs() const { return fixedTimeStep; }
	float GetMaxFrameRate() const { return 1000.f / minFrameDelta; }
	float GetFixedRate() const { return 1000.f / fixedTimeStep; }

private:
	uint32_t frameCount{};
	int minFrameDelta{};
	uint32_t internalOldTime{};
	uint32_t oldLimitedTime{};
	uint32_t newTime{};
	int smoothTime{};
	int internalDtMs{};
	int cappedDtMs{};
	int accumFixedStep{};
	int fixedTimeStep{};
	float smoothFps{};
	int delayedTime{};
	int variableOvershoot{};
	int smoothFrames{};
};

