#include "EventHub.h"
#include <algorithm>
#include <SDL.h>
#include "Updatable.h"

void EventHub::RegisterUpdatable(Updatable* updatable)
{
	updatables.push_back(updatable);
}

void EventHub::UnregisterUpdatable(Updatable* updatable)
{
	updatables.erase(std::find(updatables.begin(), updatables.end(),updatable));
}

void EventHub::Update(float deltaTime)
{
	for (auto updatable : updatables)
		updatable->Update(deltaTime);
}