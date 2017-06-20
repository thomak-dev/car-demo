#pragma once
//#include <SDL_mixer.h>
#include <string>
#include <fmod_studio.hpp>
#include "Singleton.h"

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

	//void Play(const Sound& sound);
private:
	FMOD::Studio::System* system{};
};

