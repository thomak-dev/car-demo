#pragma once
#include "RigidBody.h"
#include "math_utility.h"

namespace Tire
{
	enum Type: physx::PxU32
	{
		Normal, Slick, Winter, Spikes, Highest
	};
}

class Vehicle :	public RigidBody
{
	friend class Physics;
public:
	using RigidBody::RigidBody;
	virtual ~Vehicle();

	void Deserialize(const Json& json) override;
	void Initialize() override;

protected:
	void Update() override;
	void UpdateTransform() override;

private:
	physx::PxVehicleDrive4W* wheels{};
	float wheelMass{ 10 };
	float wheelRadius{ .5f };
	float wheelWidth{ .3f };
	float steer{ Float::Pi / 8 };
	Tire::Type type{ Tire::Normal };
	std::vector<Transform*> wheelTransforms;
	std::vector<physx::PxTransform> lastKnownWheelTransforms;
	bool airborne{};
	physx::PxVehicleDrive4WRawInputData vehicleInputData;

	void PostProcessPhysics() override;
	void BeforeVehicleUpdate(float deltaTime);
	void AfterVehicleUpdate(const physx::PxVehicleWheelQueryResult& queryResult);
};

