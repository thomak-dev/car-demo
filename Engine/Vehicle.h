#pragma once
#include "RigidBody.h"
#include "math_utility.h"

namespace Tire
{
	enum Type: physx::PxU32
	{
		Normal, Slick, Winter, Spikes, Highest
	};

	Type FromString(const std::string& str);
}

class Vehicle :	public RigidBody
{
	friend class Physics;
public:
	explicit Vehicle(Entity& entity);
	virtual ~Vehicle();

	int Deserialize(const Json& json) override;
	
	void Initialize() override;

protected:
	void Update() override;
	void UpdateTransform() override;
	void UpdateMassAndInertia() override;

private:
	const int NumWheels;
	physx::PxVehicleDrive4W* wheels{};
	float wheelMass{ 10 };
	float wheelRadius{ .5f };
	float wheelWidth{ .3f };
	float steer{ Float::Pi / 4 }; // rad
	float peakTorque{ 1000 }; // Nm
	float maxOmega{ 600 }; // rad/s
	float clutchStrength{ 10 }; // kg * m² / s 
	float switchTime{ 0.5f };
	float autoBoxLatency{ 2 };
	float springFrequency{ 5 }; // typically 5-10
	float dampingRatio{ 1 }; // underdamped 0.8..1.2 overdamped
	float maxDroop{ 0 }; // positive values or zero for automatic computation
	float maxCompression{ 0.2f };
	float forceAppPointOffset{ -0.2f };
	float camberAngleAtRest = 0.0;
	float camberAngleAtMaxDroop = 0.1f;
	float camberAngleAtMaxCompression = -0.1f;
	Tire::Type tireType{ Tire::Normal };
	std::vector<Transform*> wheelTransforms;
	std::vector<physx::PxTransform> lastKnownWheelTransforms;
	bool airborne{};
	physx::PxVehicleDrive4WRawInputData vehicleInputData;
	bool forward{ true };
	physx::PxRigidDynamic* rigidDynamic{};

	void PostProcessPhysics() override;
	void BeforeVehicleUpdate(float deltaTime);
	void AfterVehicleUpdate(const physx::PxVehicleWheelQueryResult& queryResult);
	static physx::PxConvexMesh* CreateWheelMesh(float radius, float width);

	void ConfigureDifferential(physx::PxVehicleDriveSimData4W& simData);
	void ConfigureEngine(physx::PxVehicleDriveSimData4W& simData);
	void ConfigureGears(physx::PxVehicleDriveSimData4W& simData);
	void ConfigureClutch(physx::PxVehicleDriveSimData4W& simData);
	void ConfigureAckermannCorrection(physx::PxVehicleDriveSimData4W& simData, const physx::PxVehicleWheelsSimData& wheelsSimData);
	void ConfigureAutoBox(physx::PxVehicleDriveSimData4W& simData);
	void SetUpEachWheel(physx::PxVehicleWheelsSimData& simData, int numWheels);
	void SetWheelShapes(int numWheels);

};

