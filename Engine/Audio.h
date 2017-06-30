#pragma once
#include <vector>
#include <memory>
#include <fmod_studio.hpp>
#include <unordered_map>
#include "Singleton.h"

class Entity;
class AudioListener;

#define FMOD_CHECK(expr){ FMOD_RESULT result;\
	if((result = expr) != FMOD_OK){\
		std::cout << FMOD_ErrorString(result) << std::endl;\
		PRO_ASSERT(false);\
	}}

class Audio : public Singleton<Audio>
{
	friend class AudioSource;
public:

	class Bank
	{
		friend class Audio;
	public:
		explicit Bank(const std::string& path);
		~Bank();

		FMOD::Studio::Bank* studioBank{};
	private:
		static Audio* audio;
	};

	Audio();
	virtual ~Audio();

	void Update();

	void RegisterListener(AudioListener* listener);
	void UnregisterListener(AudioListener* listener);
	FMOD::Studio::EventInstance* LoadEvent(const std::string& name);

private:
	FMOD::Studio::System* system{};
	std::shared_ptr<Bank> stringsBank;
	std::unordered_map<std::string, FMOD::Studio::EventDescription*> eventDescriptions;
	std::vector<std::shared_ptr<Bank>> banks;
	std::vector<AudioListener*> listeners;
	void UpdateListener(int index, AudioListener* listener);

	static void Get3DAttributesOfEntity(const Entity& entity, FMOD_3D_ATTRIBUTES& attribs);
};

