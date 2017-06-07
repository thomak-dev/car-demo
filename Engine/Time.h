#pragma once
#include <cstdint>
#include "Singleton.h"

/**
 * \brief 
 * Simple helper class to deal with time and framerate in main loops
 * Supports fixed and variable time step, as well as framerate limiting
 */
class Time : public Singleton<Time>
{
public:
	explicit Time(int maxFrameRate = 144, int fixedRate = 60);
	bool BeginNewFrame();
	float GetSmoothFps() const { return smoothFps; }
	void SetSmoothCount(int count) { smoothFrames = count; }
	void SetStrict(bool value) { strictLimiting = value; }
	bool IsStrict() const { return strictLimiting; }
	int GetSmoothCount() const { return smoothFrames; }
	float GetDeltaTime() const { return dtMs / 1000.f; }
	int GetDeltaTimeMs() const { return dtMs; }
	float GetTime() const { return 1000.f / newTime; }
	uint32_t GetTimeMs() const { return newTime; }
	uint32_t GetFrameCount() const { return frameCount; }
	float GetFixedTimeStep() const { return fixedTimeStep / 1000.0f; }
	int GetFixedTimeStepMs() const { return fixedTimeStep / 1000.0f; }

private:
	bool strictLimiting{};
	uint32_t frameCount{};
	int maxFrameRate{};
	int smoothFrames{};
	int minFrameDelta{};
	uint32_t oldTime{};
	uint32_t newTime{};
	int smoothTime{};
	int dtMs{};
	int accumFixedStep{};
	int accumDelay{};
	int fixedTimeStep{};
	float smoothFps{};
};

