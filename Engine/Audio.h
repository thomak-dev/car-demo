#pragma once
//#include <SDL_mixer.h>
#include <string>
#include <vector>
#include <memory>
#include <fmod_studio.hpp>
#include <unordered_map>
#include "json.h"
#include "Singleton.h"

class Entity;
class AudioListener;

#define FMOD_CHECK(expr){ FMOD_RESULT result;\
	if((result = expr) != FMOD_OK){\
		std::cout << FMOD_ErrorString(result) << std::endl;\
		SDL_assert(false);\
	}}

class Audio : public Singleton<Audio>
{
	friend class AudioSource;
public:
	//class Sound
	//{
	//	friend class Audio;
	//public:
	//	explicit Sound(const std::string& path);
	//	~Sound();
	//private:
	//	Mix_Chunk* chunk{};

	//	
	//};

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

	//void Play(const Sound& sound);
private:
	FMOD::Studio::System* system{};
	std::shared_ptr<Bank> stringsBank;
	std::unordered_map<std::string, FMOD::Studio::EventDescription*> eventDescriptions;
	std::vector<std::shared_ptr<Bank>> banks;
	std::vector<AudioListener*> listeners;
	void UpdateListener(int index, AudioListener* listener);

	static void Get3DAttributesOfEntity(const Entity& entity, FMOD_3D_ATTRIBUTES& attribs);
};

