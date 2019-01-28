#include "input.h"
#include <glm/vec2.hpp>
#include <unordered_set>
#include "math_utility.h"

static std::unordered_set<SDL_Keycode> keysWhichWentDown;
static std::unordered_set<SDL_Keycode> keysWhichWentUp;
static std::unordered_set<Uint8> mouseButtonsWhichWentDown;
static std::unordered_set<Uint8> mouseButtonsWhichWentUp;
static bool mouseButtonsThatAreDown[256];
static glm::vec2 mouseWheelDelta;

void PrepareInput()
{
	keysWhichWentDown.clear();
	keysWhichWentUp.clear();
	mouseButtonsWhichWentDown.clear();
	mouseButtonsWhichWentUp.clear();
	mouseWheelDelta.x = 0;
	mouseWheelDelta.y = 0;
}

void UpdateInput(const SDL_Event& event)
{
	if (event.type == SDL_KEYDOWN && !event.key.repeat)
		keysWhichWentDown.insert(event.key.keysym.sym);
	if (event.type == SDL_KEYUP)
		keysWhichWentUp.insert(event.key.keysym.sym);
	if (event.type == SDL_MOUSEBUTTONDOWN)
	{
		mouseButtonsWhichWentDown.insert(event.button.button);
		mouseButtonsThatAreDown[event.button.button] = true;
	}
	if (event.type == SDL_MOUSEBUTTONUP)
	{
		mouseButtonsWhichWentUp.insert(event.button.button);
		mouseButtonsThatAreDown[event.button.button] = false;
	}
	if (event.type == SDL_MOUSEWHEEL)
	{
		mouseWheelDelta.x = NarrowCast<float>(event.wheel.x);
		mouseWheelDelta.y = NarrowCast<float>((event.wheel.type == SDL_MOUSEWHEEL_FLIPPED ? -1 : 1) * event.wheel.y);
	}
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

bool MouseButtonIsDown(Uint8 button)
{
	return mouseButtonsThatAreDown[button];
}

bool MouseButtonWentDown(Uint8 button)
{
	return mouseButtonsWhichWentDown.count(button);
}

bool MouseButtonWentUp(Uint8 button)
{
	return mouseButtonsWhichWentUp.count(button);
}

glm::vec2 GetMouseWheelDelta()
{
	return mouseWheelDelta;
}

glm::vec2 GetMouseMotion()
{
	int x;
	int y;
	SDL_GetRelativeMouseState(&x, &y);
	return glm::vec2(x, y);
}
