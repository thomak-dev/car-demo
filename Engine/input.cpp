#include "input.h"
#include <glm/vec2.hpp>
#include <unordered_set>

static std::unordered_set<SDL_Keycode> keysWhichWentDown;
static std::unordered_set<SDL_Keycode> keysWhichWentUp;

void TickInput()
{
	keysWhichWentDown.clear();
	keysWhichWentUp.clear();
}

void UpdateInput(const SDL_Event& event)
{
	if (event.type == SDL_KEYDOWN && !event.key.repeat)
		keysWhichWentDown.insert(event.key.keysym.sym);
	if (event.type == SDL_KEYUP)
		keysWhichWentUp.insert(event.key.keysym.sym);
}

bool KeyWentDown(SDL_Keycode key)
{
	return keysWhichWentDown.count(key);
}

bool KeyIsDown(SDL_Keycode key)
{
	return SDL_GetKeyboardState(nullptr)[SDL_GetScancodeFromKey(key)];
}

bool KeyIsDown(SDL_Keymod key)
{
	return SDL_GetModState() & key;
}

bool KeyWentUp(SDL_Keycode key)
{
	return keysWhichWentUp.count(key);
}

glm::vec2 GetMouseMotion()
{
	int x;
	int y;
	SDL_GetRelativeMouseState(&x, &y);
	return glm::vec2(x, y);
}
