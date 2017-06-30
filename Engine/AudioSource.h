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
	void SetParameter(const std::string& name, float value);

private:
	static Audio* audio;
	using Parameter = std::pair<const std::string, std::pair<float, bool>>;
	std::unordered_multimap<std::string, FMOD::Studio::EventInstance*> events;
	
	std::unordered_map<std::string, std::pair<float, bool>> parameters;
	float volume{ 1 };

	void Play(const std::string& eventName, FMOD::Studio::EventInstance* eventInstance, float volume);
	void Update(float deltaTime);
	static void ApplyParam(Parameter& param, FMOD::Studio::EventInstance& event);
};

