#include "Updatable.h"
#include "EventHub.h"

Updatable::Updatable()
{
	EventHub::Instance().RegisterUpdatable(this);
}

Updatable::~Updatable()
{
	EventHub::Instance().UnregisterUpdatable(this);
}
