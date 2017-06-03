#pragma once
#include <string>
#include <vector>
#include <SDL.h>
#include "core.h"
#include "Singleton.h"

class GameWindow : public virtual Singleton<GameWindow>
{
	DELETE_COPY_MOVE(GameWindow)
public:
	GameWindow(const std::string& title, int width, int height);
	~GameWindow();

	void Swap();

	float Width() const
	{
		int w;
		SDL_GetWindowSize(window, &w, nullptr);
		return static_cast<float>(w);
	}
	float Height() const
	{
		int h;
		SDL_GetWindowSize(window, nullptr, &h);
		return static_cast<float>(h);
	}
private:
	SDL_Window* window;
	SDL_GLContext context;
	std::vector<std::string> extensions;
};

