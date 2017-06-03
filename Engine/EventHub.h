#pragma once
#include <vector>
#include "core.h"
#include "Singleton.h"

class Updatable;

class EventHub : public virtual Singleton<EventHub>
{
	DELETE_COPY_MOVE(EventHub)
public:
	EventHub() = default;
	virtual ~EventHub() = default;
	
	void RegisterUpdatable(Updatable* updatable);
	void UnregisterUpdatable(Updatable* updatable);
	void Update(float deltaTime);

private:
	std::vector<Updatable*> updatables;
};

