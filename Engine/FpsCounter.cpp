#include "FpsCounter.h"
#include <sstream>
#include <iomanip>
#include "Entity.h"
#include "Text.h"
#include "Time.h"
#include "messages.h"

void FpsCounter::Initialize()
{
	Component::Initialize();
	text = entity.GetComponent<Text>();
}

void FpsCounter::OnMessageReceived(Entity* origin, Message* message)
{
	HandleUpdate(message, UpdateFunctionDt{ std::bind(&FpsCounter::Update, this, std::placeholders::_1) });
}

void FpsCounter::Update(float deltaTime)
{
	std::stringstream sstrm{};
	sstrm << "FPS: ";
	sstrm << std::fixed << std::setprecision(1) << std::setw(5) << Time::Instance().GetSmoothFps();
	text->content = sstrm.str();
}
