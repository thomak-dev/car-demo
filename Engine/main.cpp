#include <iostream>
#include <iomanip>
#include <SDL.h>
#include <chrono>
#include <objbase.h>
#include "core.h"
#include "GameWindow.h"
#include "Renderer.h"
#include "Time.h"
#include "ResourceManager.h"
#include "Entity.h"
#include "input.h"
#include "Physics.h"
#include "messages.h"
#include "Audio.h"

int main(int argc, char* argv[])
{	
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	Initialize();
	GameWindow gameWindow{"Engine", 1280, 720};
	ResourceManager resourceManager;
	Renderer renderer;
	Physics physics;
	Audio audio;

	Time time{ 200, 60 };
	
	std::shared_ptr<Entity> root{ Entity::Instantiate(LOAD(Json, "root.prefab"), nullptr) };
	root->Initialize();

	SDL_Event event;
	SDL_GetRelativeMouseState(nullptr, nullptr);

	bool quit = false;
	while (!quit)
	{
		bool isFixedTimeStep;
		bool isVariable;
		time.BusyTick(isVariable, isFixedTimeStep);

		if(isVariable)
		{
			PrepareInput();

			while (SDL_PollEvent(&event))
			{
				UpdateInput(event);
				if (event.type == SDL_QUIT)
					quit = true;
			}
			if (KeyWentDown(SDLK_ESCAPE))
				quit = true;
			if (KeyWentDown(SDLK_F1))
				physics.SetVisualize(!physics.Visualize());
		}

		if (isFixedTimeStep)
		{
			float timeStep = time.GetFixedTimeStep();
			if (time.GetFixedFrameCount() > 1)
				physics.Await();
			physics.Step(timeStep);
			UpdateMessage fixedUpdate{ timeStep, true };
			root->SendMessageDown(&fixedUpdate);
		}
		if (isVariable)
		{
			float deltaTime = time.GetDeltaTime();
			UpdateMessage update{ deltaTime, false };
			root->SendMessageDown(&update);
			audio.Update();
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, RenderDebugId::Frame, -1, "New Frame");
			renderer.Render();
			gameWindow.Swap();
			glPopDebugGroup();
		}
	}

	CoUninitialize();
	return 0;
}

