#pragma once
#include <SDL.h>
#include <glm/fwd.hpp>

void TickInput();
void UpdateInput(const SDL_Event& event);
bool KeyWentDown(SDL_Keycode key);
bool KeyIsDown(SDL_Keycode key);
bool KeyIsDown(SDL_Keymod key);
bool KeyWentUp(SDL_Keycode key);
glm::vec2 GetMouseMotion();