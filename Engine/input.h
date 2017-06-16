#pragma once
#include <SDL.h>
#include <glm/fwd.hpp>

void PrepareInput();
void UpdateInput(const SDL_Event& event);
bool KeyWentDown(SDL_Keycode key);
bool KeyIsDown(SDL_Keycode key);
bool KeyIsDown(SDL_Keymod key);
bool KeyWentUp(SDL_Keycode key);
bool MouseButtonIsDown(Uint8 button);
bool MouseButtonWentDown(Uint8 button);
bool MouseButtonWentUp(Uint8 button);
glm::vec2 GetMouseWheelDelta();
glm::vec2 GetMouseMotion();
