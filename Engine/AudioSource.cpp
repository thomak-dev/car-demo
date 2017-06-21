#include "AudioSource.h"
#include <iostream>
#include <fmod_errors.h>
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
	FMOD::Studio::EventInstance* instance = audio->LoadEvent(eventName);
	Play(eventName, instance, volume);
}

void AudioSource::Play(const std::string& eventName, FMOD::Studio::EventInstance* eventInstance, float volume)
{
	FMOD_CHECK(eventInstance->setVolume(this->volume * volume));
	FMOD_CHECK(eventInstance->start());
	events.insert(std::make_pair(eventName, eventInstance));
}

int AudioSource::Deserialize(const Json& json)
{
	int count = Component::Deserialize(json);
	if (json.HasMember("volume") && ++count)
		volume = json["volume"].GetFloat();
	if(json.HasMember("events") && ++count)
	{
		for (auto& event : json["events"].GetArray())
		{
			Play(event.GetString());
		}
	}
	if(json.HasMember("parameters") && ++count)
	{
		for (auto& param : json["parameters"].GetObject())
			SetParameter(param.name.GetString(), param.value.GetFloat());
	}
	return count;
}

void AudioSource::Initialize()
{
	for(auto& event : events)
	{
		for (auto& param : parameters)
			ApplyParam(param, *event.second);
	}
}

void AudioSource::OnMessageReceived(Entity* origin, Message* message)
{
	HandleUpdate(message, UpdateFunctionDt{ std::bind(&AudioSource::Update, this, std::placeholders::_1) });
}

void AudioSource::SetParameter(const std::string& name, float value)
{
	auto& elem = parameters[name];
	elem.first = value;
	elem.second = true;
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
		{
			for (auto& param : parameters)
				if (param.second.second)
					ApplyParam(param, *it->second);

			FMOD_3D_ATTRIBUTES attribs;
			Audio::Get3DAttributesOfEntity(entity, attribs);
			FMOD_CHECK(it->second->set3DAttributes(&attribs))
			++it;
		}
	}
}

void AudioSource::ApplyParam(Parameter& param, FMOD::Studio::EventInstance& event)
{
	FMOD_CHECK(event.setParameterValue(param.first.c_str(), param.second.first))
	param.second.second = false;
}
