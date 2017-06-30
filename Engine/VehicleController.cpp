#include "VehicleController.h"
#include "Entity.h"
#include "Vehicle.h"
#include "AudioSource.h"
#include "input.h"

void VehicleController::Initialize()
{
	vehicle = entity.GetComponent<Vehicle>();
	audioSource = entity.GetComponent<AudioSource>();
	transform = entity.GetComponent<Transform>();
}

void VehicleController::Update(float deltaTime)
{
	float rpm = vehicle->GetRpm();
	audioSource->SetParameter("RPM", rpm);

	if (length(vehicle->Velocity()) < 0.3)
	{
		if (KeyIsDown(SDLK_s) && forward)
		{
			forward = false;
			vehicle->SetGear(physx::PxVehicleGearsData::eREVERSE);
		}
		else if (KeyIsDown(SDLK_w) && !forward)
		{
			forward = true;
			vehicle->SetGear(physx::PxVehicleGearsData::eFIRST);
		}
	}

	vehicle->accelerate = KeyIsDown(forward ? SDLK_w : SDLK_s);
	vehicle->brake = KeyIsDown(forward ? SDLK_s : SDLK_w);
	vehicle->handbrake = KeyIsDown(SDLK_SPACE);
	vehicle->steerRight = KeyIsDown(SDLK_d);
	vehicle->steerLeft = KeyIsDown(SDLK_a);

	if (KeyWentDown(SDLK_r) && dot(transform->Up(), Vector3::Up) < 0.5f && abs(vehicle->Velocity().y) < 0.1f)
	{
		vehicle->AddForce(glm::vec3(0, vehicle->Mass() * 10, 0), physx::PxForceMode::eIMPULSE);
		vehicle->AddTorque(transform->WorldRotation() * glm::vec3(0, 0, vehicle->MassSpaceInertiaTensor().z * 2.f), physx::PxForceMode::eIMPULSE);
	}
}
