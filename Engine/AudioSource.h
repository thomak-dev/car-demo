#pragma once
#include <unordered_map>
#include <fmod_studio.hpp>
#include "Component.h"

class Audio;

class AudioSource : public Component
{
	friend class Audio;
	DELETE_COPY_MOVE(AudioSource)
public:
	using Component::Component;
	virtual ~AudioSource();

	void Play(const std::string& eventName, float volume = 1);
	int Deserialize(const Json& json) override;
	void Initialize() override;
	void OnMessageReceived(Entity* origin, Message* message) override;
private:
	static Audio* audio;
	std::unordered_multimap<std::string, FMOD::Studio::EventInstance*> events;
	std::unordered_map<std::string, float> parameters;
	float volume{ 1 };

	void Update(float deltaTime);
};

