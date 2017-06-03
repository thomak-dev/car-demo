#pragma once
class Updatable
{
public:
	Updatable();
	virtual ~Updatable();

	virtual void Update(float deltaTime) = 0;
};

