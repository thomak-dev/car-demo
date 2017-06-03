#pragma once
#include <cstdint>
#include "Singleton.h"

class Time : public Singleton<Time>
{
public:
	Time(int maxFrameRate);
	void BeginNewFrame();
	float GetFps() const;
	float GetDeltaTime() const;
	float GetTime() const;
	uint32_t GetTimeMs() const;
	int GetFrameCount() const;
private:
	int frameCount{};
	int maxFrameRate{};
	const int smoothFrames{};
	uint32_t minFrameDelta{};
	uint32_t oldTime{};
	uint32_t newTime{};
	uint32_t smoothTime{};
	uint32_t dtMs{};
	float smoothFps{};
};

