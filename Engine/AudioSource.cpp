#include "AudioSource.h"
#include "Audio.h"
#include "messages.h"

Audio* AudioSource::audio;

AudioSource::~AudioSource()
{
	for (auto& event : events)
		event.second->release();
}

void AudioSource::Play(const std::string& eventName, float volume)
{
	FMOD::Studio::EventDescription* desc;
	audio->system->getEvent(eventName.c_str(), &desc);
	FMOD::Studio::EventInstance* instance;
	desc->createInstance(&instance);
	instance->setVolume(this->volume * volume);
	instance->start();
	events.insert(std::make_pair(eventName, instance));
}

int AudioSource::Deserialize(const Json& json)
{
	int count = Component::Deserialize(json);
	if (json.HasMember("volume") && ++count)
		volume = json["volume"].GetFloat();
	if(json.HasMember("events") && ++count)
	{
		for (auto& event : json["events"].GetArray())
			Play(event.GetString());
	}
	if(json.HasMember("parameters") && ++count)
	{
		for (auto& param : json["parameters"].GetObject())
			parameters[param.name.GetString()] = param.value.GetFloat();
	}
	return count;
}

void AudioSource::Initialize()
{
	for(auto& event : events)
	{
		for (auto& param : parameters)
			event.second->setParameterValue(param.first.c_str(), param.second);
	}
}

void AudioSource::OnMessageReceived(Entity* origin, Message* message)
{
	HandleUpdate(message, UpdateFunctionDt{ std::bind(&AudioSource::Update, this, std::placeholders::_1) });
}

void AudioSource::Update(float deltaTime)
{
	for (auto it = events.begin(); it != events.end();)
	{
		FMOD_STUDIO_PLAYBACK_STATE state;
		it->second->getPlaybackState(&state);
		if (state == FMOD_STUDIO_PLAYBACK_STOPPED)
		{
			it->second->release();
			it = events.erase(it);
		}
		else
			++it;
	}
}
