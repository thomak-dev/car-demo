#include "VehicleController.h"
#include "Entity.h"
#include "Vehicle.h"
#include "AudioSource.h"

void VehicleController::Initialize()
{
	vehicle = entity.GetComponent<Vehicle>();
	audioSource = entity.GetComponent<AudioSource>();
}

void VehicleController::Update(float deltaTime)
{
	float rpm = vehicle->GetRpm();
	audioSource->SetParameter("RPM", rpm);
}
