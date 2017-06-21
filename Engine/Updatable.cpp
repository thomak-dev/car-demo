#include "Updatable.h"
#include "messages.h"

void Updatable::OnMessageReceived(Entity* origin, Message* message)
{
	HandleUpdate(message, UpdateFunctionDt{ std::bind(&Updatable::Update, this, std::placeholders::_1) });
}
