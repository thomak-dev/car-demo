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

	void Initialize() override;

private:
	physx::PxVehicleDrive4W* wheels{};
	float wheelMass{ 10 };
	float wheelRadius{ .5f };
	float wheelWidth{ .3f };
	float steer{ Float::Pi / 8 };
	Tire::Type type{ Tire::Normal };

	void PostProcessPhysics() override;
};

