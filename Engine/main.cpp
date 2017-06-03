#include <iostream>
#include <iomanip>
#include <SDL.h>
#include "core.h"
#include "GameWindow.h"
#include "Renderer.h"
#include "Time.h"
#include "ResourceManager.h"
#include "Entity.h"
#include "input.h"
#include "Physics.h"
#include "messages.h"

int main(int argc, char* argv[])
{
	Initialize();
	GameWindow gameWindow{"Engine", 1280, 720};
	ResourceManager resourceManager;
	Renderer renderer;
	Physics physics;

	Time time{ 150 };

	std::shared_ptr<Entity> root{ Entity::Instantiate(resourceManager.LoadPrefab("root.prefab"), nullptr) };
	root->Initialize();

	SDL_Event event;
	SDL_GetRelativeMouseState(nullptr, nullptr);

	bool quit = false;
	while (!quit)
	{
		time.BeginNewFrame();
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, RenderDebugId::Frame, -1, "Main loop");

		TickInput();

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
				UpdateInput(event);
			if (event.type == SDL_QUIT)
				quit = true;
		}
		if (KeyWentDown(SDLK_ESCAPE))
			quit = true;
		if (KeyWentDown(SDLK_F1))
			physics.SetVisualize(!physics.Visualize());

		float deltaTime = time.GetDeltaTime();
		
		physics.Step(deltaTime);
		UpdateMessage update{ deltaTime };
		root->SendMessageDown(&update);
		renderer.Render();
		gameWindow.Swap();
		glPopDebugGroup();
	}

	return 0;
}

