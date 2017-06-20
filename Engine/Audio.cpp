#include "Audio.h"
//#include <SDL_mixer.h>
#include <iostream>
#include <fmod_studio.hpp>
#include <fmod_errors.h>
#include "AudioSource.h"
#include "core.h"

Audio* Audio::Bank::audio;

//Audio::Sound::Sound(const std::string& path)
//	:chunk{ Mix_LoadWAV(path.c_str()) }
//{
//	
//}

//Audio::Sound::~Sound()
//{
//	Mix_FreeChunk(chunk);
//}

FMOD_RESULT F_CALLBACK OnFmodError(FMOD_SYSTEM* system, FMOD_SYSTEM_CALLBACK_TYPE type, void* commanddata1, void* commanddata2, void* userdata)
{
	SDL_assert(IsThisTheMainThread());
	switch (type)
	{
	case FMOD_SYSTEM_CALLBACK_MEMORYALLOCATIONFAILED:
		std::cout 
			<< "FMOD error: FMOD_SYSTEM_CALLBACK_MEMORYALLOCATIONFAILED. Size: "
			<< reinterpret_cast<int>(commanddata2) 
			<< ". File: "
			<< static_cast<char*>(commanddata1)
			<< std::endl;
		break;
	case FMOD_SYSTEM_CALLBACK_BADDSPCONNECTION:
		{
			FMOD::DSP* source = static_cast<FMOD::DSP*>(commanddata1);
			FMOD::DSP* dest = static_cast<FMOD::DSP*>(commanddata2);

			std::cout << "FMOD error: FMOD_SYSTEM_CALLBACK_BADDSPCONNECTION. ";
			if (source)
			{
				char name[256];
				source->getInfo(name, nullptr, nullptr, nullptr, nullptr);
				std::cout << "Source: " << name << ". ";
			}
			if (dest)
			{
				char name[256];
				dest->getInfo(name, nullptr, nullptr, nullptr, nullptr);
				std::cout << "Dest: " << name;
			}
			std::cout << std::endl;
			break;
		}
	case FMOD_SYSTEM_CALLBACK_ERROR:
		{
			FMOD_ERRORCALLBACK_INFO* info = static_cast<FMOD_ERRORCALLBACK_INFO*>(commanddata1);
			std::cout
				<< "FMOD error: FMOD_SYSTEM_CALLBACK_ERROR. "
				<< FMOD_ErrorString(info->result)
				<< '\n' << "Function name: "
				<< info->functionname
				<< " Params: "
				<< info->functionparams
				<< " Instance: "
				<< info->instance
				<< " Type: "
				<< info->instancetype
				<< std::endl;
			SDL_assert(false);
			break;
		}
	case FMOD_SYSTEM_CALLBACK_DEVICELOST:
		std::cout << "FMOD error: FMOD_SYSTEM_CALLBACK_DEVICELOST" << std::endl;
		break;
	default:
		return FMOD_OK;
	}
	SDL_assert(false);
	return FMOD_OK;
}


Audio::Bank::Bank(const std::string& path)
{
	audio->system->loadBankFile(path.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &studioBank);
}

Audio::Bank::~Bank()
{
	studioBank->unload();
}

Audio::Audio()
{
	//Mix_Init(MIX_INIT_OGG);
	//bool success = Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 256) == 0;
	//SDL_assert(success);
	//Mix_AllocateChannels(32);

	FMOD_RESULT result;
	if((result = FMOD::Studio::System::create(&system)) != FMOD_OK)
		std::cout << FMOD_ErrorString(result) << std::endl;
	SDL_assert(result == FMOD_OK);

	FMOD::System* lowSystem;
	if ((result = system->getLowLevelSystem(&lowSystem)) != FMOD_OK)
		std::cout << FMOD_ErrorString(result) << std::endl;
	SDL_assert(result == FMOD_OK);
	lowSystem->setCallback(OnFmodError);
	FMOD_ADVANCEDSETTINGS advSettings{};
	advSettings.vol0virtualvol = 0.001f;
	lowSystem->setAdvancedSettings(&advSettings);
	if((result = system->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL | FMOD_INIT_VOL0_BECOMES_VIRTUAL | FMOD_INIT_3D_RIGHTHANDED, nullptr)) != FMOD_OK)
		std::cout << FMOD_ErrorString(result) << std::endl;
	SDL_assert(result == FMOD_OK);

	Bank::audio = this;
	AudioSource::audio = this;
}


Audio::~Audio()
{
	system->release();
	//Mix_CloseAudio();
	//Mix_Quit();
}

void Audio::Update()
{
	system->update();
}

//void Audio::Play(const Sound& sound)
//{
//	if (Mix_PlayChannel(-1, sound.chunk, 0) == -1)
//		std::cout << Mix_GetError() << std::endl;
//}
