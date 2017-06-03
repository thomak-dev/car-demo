#pragma once
#include <SDL.h>

// Non-lazy singleton implementation "mixin"
template <typename T>
class Singleton
{
public:
	Singleton()
	{
		SDL_assert(instance == nullptr);
		instance = this;
	}

	virtual ~Singleton()
	{
		instance = nullptr;
	}

	static T& Instance()
	{
		return *dynamic_cast<T*>(instance);
	}
private:
	static Singleton<T>* instance;
};

template<typename T>
Singleton<T>* Singleton<T>::instance{ nullptr };