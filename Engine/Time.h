#pragma once
#include <cstdint>
#include "Singleton.h"

class Time : public Singleton<Time>
{
public:
	Time(int maxFrameRate, int fixedRate);
	bool BeginNewFrame();
	float GetFps() const { return smoothFps; }
	float GetDeltaTime() const { return dtMs / 1000.f; }
	float GetTime() const { return 1000.f / newTime; }
	uint32_t GetTimeMs() const { return newTime; }
	int GetFrameCount() const { return frameCount; }
	float GetFixedTimeStep() const { return fixedTimeStep / 1000.0f; }
private:
	int frameCount{};
	int maxFrameRate{};
	const int smoothFrames{};
	uint32_t minFrameDelta{};
	uint32_t oldTime{};
	uint32_t newTime{};
	uint32_t smoothTime{};
	uint32_t dtMs{};
	uint32_t accumulated{};
	uint32_t fixedTimeStep{};
	float smoothFps{};
};

