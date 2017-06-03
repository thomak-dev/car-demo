#include "FpsCounter.h"
#include <sstream>
#include <iomanip>
#include "Entity.h"
#include "Text.h"
#include "Time.h"

void FpsCounter::Initialize()
{
	text = entity->GetComponent<Text>();
}

void FpsCounter::Update(float deltaTime)
{
	std::stringstream sstrm{};
	sstrm << "FPS: ";
	sstrm << std::setw(4) << std::round(Time::Instance().GetFps());
	text->content = sstrm.str();
}
