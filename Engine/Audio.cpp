#include "Audio.h"
//#include <SDL_mixer.h>
#include <iostream>
#include <sstream>
#include <fmod_studio.hpp>
#include <fmod_errors.h>
#include "AudioSource.h"
#include "core.h"
#include "AudioListener.h"
#include "Transform.h"
#include "Entity.h"
#include "RigidBody.h"
#include <mutex>

Audio* Audio::Bank::audio;

FMOD_RESULT F_CALL OnFmodError(FMOD_SYSTEM* system, FMOD_SYSTEM_CALLBACK_TYPE type, void* commanddata1, void* commanddata2, void* userdata)
{
	std::stringstream sstrm;
	switch (type)
	{
	case FMOD_SYSTEM_CALLBACK_MEMORYALLOCATIONFAILED:
		sstrm
			<< "FMOD error: FMOD_SYSTEM_CALLBACK_MEMORYALLOCATIONFAILED. Size: "
			<< reinterpret_cast<int>(commanddata2) 
			<< ". File: "
			<< static_cast<char*>(commanddata1)
			<< '\n';
		break;
	case FMOD_SYSTEM_CALLBACK_BADDSPCONNECTION:
		{
			FMOD::DSP* source = static_cast<FMOD::DSP*>(commanddata1);
			FMOD::DSP* dest = static_cast<FMOD::DSP*>(commanddata2);

			sstrm << "FMOD error: FMOD_SYSTEM_CALLBACK_BADDSPCONNECTION. ";
			if (source)
			{
				char name[256];
				source->getInfo(name, nullptr, nullptr, nullptr, nullptr);
				sstrm << "Source: " << name << ". ";
			}
			if (dest)
			{
				char name[256];
				dest->getInfo(name, nullptr, nullptr, nullptr, nullptr);
				sstrm << "Dest: " << name;
			}
			sstrm << '\n';
			break;
		}
	case FMOD_SYSTEM_CALLBACK_ERROR:
		{
			FMOD_ERRORCALLBACK_INFO* info = static_cast<FMOD_ERRORCALLBACK_INFO*>(commanddata1);
			if(info->instancetype == FMOD_ERRORCALLBACK_INSTANCETYPE_DSP)
				return FMOD_OK;
			sstrm
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
				<< '\n';
			break;
		}
	case FMOD_SYSTEM_CALLBACK_DEVICELOST:
		sstrm << "FMOD error: FMOD_SYSTEM_CALLBACK_DEVICELOST\n";
		break;
	default:
		return FMOD_OK;
	}

	static std::mutex mutex;
	{
		std::lock_guard<std::mutex>{mutex};
		std::cout << sstrm.str();
		std::cout.flush();
	}

	PRO_ASSERT(false);
	return FMOD_OK;
}


Audio::Bank::Bank(const std::string& path)
{
	FMOD_CHECK(audio->system->loadBankFile(path.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &studioBank));
}

Audio::Bank::~Bank()
{
	FMOD_CHECK(studioBank->unload());
}

Audio::Audio()
{
	//Mix_Init(MIX_INIT_OGG);
	//bool success = Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 256) == 0;
	//PRO_ASSERT(success);
	//Mix_AllocateChannels(32);

	FMOD_CHECK(FMOD::Studio::System::create(&system));

	FMOD::System* coreSystem;
	FMOD_CHECK(system->getCoreSystem(&coreSystem));
	//coreSystem->setCallback(OnFmodError);
	FMOD_ADVANCEDSETTINGS advSettings{};
	advSettings.vol0virtualvol = 0.001f;
	advSettings.cbSize = sizeof(FMOD_ADVANCEDSETTINGS);
	FMOD_CHECK(coreSystem->setAdvancedSettings(&advSettings));
	FMOD_CHECK(system->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL | FMOD_INIT_VOL0_BECOMES_VIRTUAL | FMOD_INIT_3D_RIGHTHANDED, nullptr));

	Bank::audio = this;
	AudioSource::audio = this;
	
	stringsBank = LOAD(Bank, "Audio/Master Bank.strings.bank");
	int stringCount;
	FMOD_CHECK(stringsBank->studioBank->getStringCount(&stringCount));
	std::vector<std::string> paths;
	for (int i = 0; i < stringCount; ++i)
	{
		int pathLength;
		FMOD_CHECK(stringsBank->studioBank->getStringInfo(i, nullptr, nullptr, 0, &pathLength));
		char* path = new char[pathLength];
		FMOD_CHECK(stringsBank->studioBank->getStringInfo(i, nullptr, path, pathLength, nullptr));
		paths.emplace_back(path);
	}

	for (auto& path : paths)
	{
		auto dotPos = path.find_last_of('.');
		if (path.substr(0, 6) == "bank:/" && (dotPos == std::string::npos || path.substr(dotPos) != ".strings"))
		{
			banks.push_back(LOAD(Bank, "Audio/" + path.substr(6) + ".bank"));
			FMOD_CHECK(banks[banks.size() - 1]->studioBank->loadSampleData());
		}
	}
}


Audio::~Audio()
{
	stringsBank = nullptr;
	banks.clear();
	ResourceManager::Instance().CleanUp();
	FMOD_CHECK(system->release());
	//Mix_CloseAudio();
	//Mix_Quit();
}

void Audio::Update()
{
	for (size_t i = 0; i < listeners.size(); ++i)
	{
		UpdateListener(i, listeners[i]);
	}
	FMOD_CHECK(system->update());
}

void Audio::RegisterListener(AudioListener* listener)
{
	listeners.push_back(listener);
	int sz = listeners.size();
	if(sz > 1)
		FMOD_CHECK(system->setNumListeners(sz));
}

void Audio::UnregisterListener(AudioListener* listener)
{
	auto found = find(listeners.begin(), listeners.end(), listener);
	if(found != listeners.end())
	{
		listeners.erase(found);
		int sz = listeners.size();
		if(sz >= 1)
			FMOD_CHECK(system->setNumListeners(sz));
	}
}

FMOD::Studio::EventInstance* Audio::LoadEvent(const std::string& name)
{
	auto found = eventDescriptions.find(name);
	FMOD::Studio::EventInstance* instance;
	FMOD::Studio::EventDescription* desc;
	if(found != eventDescriptions.end())
	{
		desc = found->second;
	}
	else
	{
		FMOD_CHECK(system->getEvent(name.c_str(), &desc));
		FMOD_CHECK(desc->loadSampleData());
		eventDescriptions.insert(found, std::make_pair(name, desc));
	}
	FMOD_CHECK(desc->createInstance(&instance));
	return instance;
}

void Audio::UpdateListener(int index, AudioListener* listener)
{
	FMOD_3D_ATTRIBUTES attribs;
	Entity& entity = listener->GetEntity();
	Get3DAttributesOfEntity(entity, attribs);

	FMOD_CHECK(system->setListenerAttributes(index, &attribs));
}

void Audio::Get3DAttributesOfEntity(const Entity& entity, FMOD_3D_ATTRIBUTES& attribs)
{
	Transform* transform = entity.GetComponent<Transform>();
	auto pos = transform->WorldPosition();
	auto forward = transform->Forward();
	auto up = transform->Up();
	attribs.position.x = pos.x;
	attribs.position.y = pos.y;
	attribs.position.z = pos.z;
	attribs.forward.x = forward.x;
	attribs.forward.y = forward.y;
	attribs.forward.z = forward.z;
	attribs.up.x = up.x;
	attribs.up.y = up.y;
	attribs.up.z = up.z;
	RigidBody* rigidBody;
	glm::vec3 velocity{ glm::uninitialize };
	if ((rigidBody = entity.GetComponentDerivedFrom<RigidBody>()))
		velocity = rigidBody->Velocity();
	else
		velocity = transform->GetVelocity();
	attribs.velocity.x = velocity.x;
	attribs.velocity.y = velocity.y;
	attribs.velocity.z = velocity.z;
}