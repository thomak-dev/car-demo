#pragma once
#include "Updatable.h"
#include "core.h"
#include "AudioSource.h"

class Vehicle;

class VehicleController : public Updatable
{
	DELETE_COPY_MOVE(VehicleController)
public:
	using Updatable::Updatable;
	virtual ~VehicleController() = default;

	void Initialize() override;

private:
	Vehicle* vehicle{};
	AudioSource* audioSource{};

	void Update(float deltaTime) override;
};

