#include "AudioListener.h"
#include "Audio.h"

AudioListener::~AudioListener()
{
	Audio::Instance().UnregisterListener(this);
}

void AudioListener::Initialize()
{
	Audio::Instance().RegisterListener(this);
}
