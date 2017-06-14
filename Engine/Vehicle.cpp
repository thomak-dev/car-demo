#include "Vehicle.h"
#include <vehicle/PxVehicleSDK.h>
#include <PxTkStream.h>
#include <vehicle/PxVehicleUtil.h>
#include "input.h"
#include "Entity.h"
#include "Transform.h"
#include "math_utility.h"
#include "Physics.h"
#include "Wheel.h"

using namespace physx;

Vehicle::~Vehicle()
{
	wheels->free();
}

void Vehicle::Deserialize(const Json& json)
{
	RigidBody::Deserialize(json);
	if (json.HasMember("wheel_radius"))
		wheelRadius = json["wheel_radius"].GetFloat();
	if (json.HasMember("wheel_width"))
		wheelWidth = json["wheel_width"].GetFloat();
	if (json.HasMember("wheel_mass"))
		wheelMass = json["wheel_mass"].GetFloat();
}

void Vehicle::Initialize()
{
	Component::Initialize();
	const PxU32 numWheels = 4;

	transform = entity->GetComponent<Transform>();
	PxTransform pTrans{ ToPxMat44(transform->WorldMatrix()) };
	PxRigidDynamic* dynamicActor = physics->backend->createRigidDynamic(pTrans.getNormalized());
	rigidActor = dynamicActor;

	PxVec3 wheelOffsets[numWheels];
	//const auto& children = entity->Children();
	//for (int i = 0; i < children.size(); ++i)
	//{
	//	if (children[i]->flags & EntityFlags::Wheel)
	//		wheelOffsets[wheelsFound++] = ToPxVec3(children[i]->GetComponent<Transform>()->Position());		
	//}
	std::vector<Wheel*> wheelComponents;
	entity->GetComponentsInDescendants<Wheel>(std::back_inserter(wheelComponents));
	SDL_assert(wheelComponents.size() >= 4);
	std::sort(wheelComponents.begin(), wheelComponents.end(), [](Wheel* a, Wheel* b) { return a->GetIndex() < b->GetIndex(); });
	for (int i = 0; i < numWheels; ++i)
	{
		wheelTransforms.push_back(wheelComponents[i]->GetEntity()->GetComponent<Transform>());
		PxTransform globalPose{ ToPxMat44(wheelTransforms[i]->WorldMatrix()) };
		lastKnownWheelTransforms.push_back(globalPose.getNormalized());
		wheelOffsets[i] = ToPxVec3(wheelTransforms[i]->Position());
	}

	PxFilterData wheelFilterData;
	wheelFilterData.word0 = EntityFlags::Wheel;

	std::vector<PxVec3> wheelVerts;
	float angle = 0;
	const int circlePoints = 32;
	for (int i = 0; i < circlePoints; ++i)
	{
		float x = cos(angle) * wheelRadius;
		float y = sin(angle) * wheelRadius;
		wheelVerts.emplace_back(wheelWidth / 2, y, x);
		wheelVerts.emplace_back(-wheelWidth / 2, y, x);
		angle += 2 * Float::Pi / circlePoints;
	}

	PxConvexMesh* convexMesh = PxToolkit::createConvexMeshSafe(*physics->backend, *physics->cooking, wheelVerts.data(), 2 * circlePoints, PxConvexFlag::eCOMPUTE_CONVEX, 64);

	//Add all the wheel shapes to the actor.
	for (PxU32 i = 0; i < numWheels; i++)
	{
		PxConvexMeshGeometry geom{ convexMesh };
		PxShape* wheelShape = PxRigidActorExt::createExclusiveShape(*dynamicActor, geom, *physics->defaultMaterial);
		wheelShape->setQueryFilterData(wheelFilterData);
		wheelShape->setSimulationFilterData(wheelFilterData);
		wheelShape->setLocalPose(PxTransform(PxIdentity));
	}
	convexMesh->release();

	filterData.word0 |= EntityFlags::Chassis;
	SetShapeInternal(shapeType);
	
	float mass = density * 2 * halfSize.x * 2 * halfSize.y * 2 * halfSize.z;
	dynamicActor->setMass(mass);
	dynamicActor->setMassSpaceInertiaTensor(PxVec3(
		(halfSize.y * halfSize.y + halfSize.z * halfSize.z) * mass / 12,
		(halfSize.x * halfSize.x + halfSize.z * halfSize.z) * mass / 12,
		(halfSize.x * halfSize.x + halfSize.y * halfSize.y) * mass / 12
	));
	dynamicActor->setCMassLocalPose(PxTransform(ToPxVec3(offset)));
	


	PxVehicleWheelsSimData* wheelsSimData = PxVehicleWheelsSimData::allocate(numWheels);
	//wheelsSimData->setSubStepCount(5, 3, 2);
	PxVehicleWheelData wheelsData[numWheels];
	PxVehicleTireData tires[numWheels];
	PxVehicleSuspensionData suspensions[numWheels];
	PxVec3 suspTravelDirections[numWheels];
	PxVec3 wheelCentreCMOffsets[numWheels];
	PxVec3 suspForceAppCMOffsets[numWheels];
	PxVec3 tireForceAppCMOffsets[numWheels];

	PxF32 suspSprungMasses[numWheels];
	PxVehicleComputeSprungMasses(numWheels, wheelOffsets, dynamicActor->getCMassLocalPose().p, dynamicActor->getMass(), 1, suspSprungMasses);

	PxFilterData qryFilterData;
	qryFilterData.word1 = EntityFlags::Default;

	wheelsData[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxSteer = steer;
	wheelsData[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mMaxSteer = steer;
	wheelsData[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mMaxHandBrakeTorque = 3000.f;
	wheelsData[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mMaxHandBrakeTorque = 3000.f;

	for (int i = 0; i < numWheels; ++i)
	{
		wheelsData[i].mMOI = 0.5f * wheelMass * wheelRadius * wheelRadius;
		wheelsData[i].mMass = wheelMass;
		wheelsData[i].mWidth = wheelWidth;
		wheelsData[i].mRadius = wheelRadius;

		tires[i].mType = type;
		const float natFreq = 5; // 5 - 10 maybe
		const float dampingRatio = 1.1f; // 0.8 underdamped .. 1.2 overdamped
		suspensions[i].mMaxCompression = 0.22f;
		suspensions[i].mSpringStrength = natFreq * natFreq * suspSprungMasses[i];
		suspensions[i].mSpringDamperRate = dampingRatio * 2 * sqrt(suspensions[i].mSpringStrength * suspSprungMasses[i]);
		suspensions[i].mSprungMass = suspSprungMasses[i];
		suspensions[i].mMaxDroop = suspSprungMasses[i] * 9.81f / suspensions[i].mSpringStrength;

		const PxF32 camberAngleAtRest = 0.0;
		const PxF32 camberAngleAtMaxDroop = -0.05f;
		const PxF32 camberAngleAtMaxCompression = 0.05f;

		suspensions[i + 0].mCamberAtRest = camberAngleAtRest * (1 - (i % 2) * 2);
		suspensions[i + 0].mCamberAtMaxDroop = camberAngleAtMaxDroop * (1 - (i % 2) * 2);
		suspensions[i + 0].mCamberAtMaxCompression = camberAngleAtMaxCompression * (1 - (i % 2) * 2);

		suspTravelDirections[i] = PxVec3(0, -1, 0);

		wheelCentreCMOffsets[i] = wheelOffsets[i] - dynamicActor->getCMassLocalPose().p;

		suspForceAppCMOffsets[i] = PxVec3(wheelCentreCMOffsets[i].x, -0.1f, wheelCentreCMOffsets[i].z);
		tireForceAppCMOffsets[i] = PxVec3(wheelCentreCMOffsets[i].x, -0.1f, wheelCentreCMOffsets[i].z);

		wheelsSimData->setWheelData(i, wheelsData[i]);
		wheelsSimData->setTireData(i, tires[i]);
		wheelsSimData->setSuspensionData(i, suspensions[i]);
		wheelsSimData->setSuspTravelDirection(i, suspTravelDirections[i]);
		wheelsSimData->setWheelCentreOffset(i, wheelCentreCMOffsets[i]);
		wheelsSimData->setSuspForceAppPointOffset(i, suspForceAppCMOffsets[i]);
		wheelsSimData->setTireForceAppPointOffset(i, tireForceAppCMOffsets[i]);
		wheelsSimData->setSceneQueryFilterData(i, qryFilterData);
		wheelsSimData->setWheelShapeMapping(i, i);
	}


	

	PxVehicleDriveSimData4W driveSimData;
	PxVehicleDifferential4WData diff;
	diff.mType = PxVehicleDifferential4WData::eDIFF_TYPE_OPEN_FRONTWD;
	driveSimData.setDiffData(diff);

	PxVehicleEngineData engine;
	engine.mPeakTorque = 1200.0f;
	engine.mMaxOmega = 300.0f; //approx 6000 rpm
	driveSimData.setEngineData(engine);

	PxVehicleGearsData gears;
	gears.mSwitchTime = 0.5f;
	driveSimData.setGearsData(gears);
	

	PxVehicleClutchData clutch;
	clutch.mStrength = 10.0f;
	driveSimData.setClutchData(clutch);

	PxVehicleAckermannGeometryData ackermann;
	ackermann.mAccuracy = 1.0f;
	ackermann.mAxleSeparation = wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_LEFT).z - wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_LEFT).z;
	ackermann.mFrontWidth =	wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_LEFT).x - wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_RIGHT).x;
	ackermann.mRearWidth = wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_LEFT).x - wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_RIGHT).x;
	driveSimData.setAckermannGeometryData(ackermann);



	wheels = PxVehicleDrive4W::allocate(numWheels);
	wheels->setup(physics->backend, dynamicActor, *wheelsSimData, driveSimData, numWheels - 4);
	wheelsSimData->free();
	wheels->mDriveDynData.setToRestState();
	wheels->mDriveDynData.setUseAutoGears(true);
	rigidActor->userData = this;
	lastPhysicsPosition = transform->WorldPosition();
	lastPhysicsRotation = transform->WorldRotation();
	physics->RegisterRigidBody(this);
}

void Vehicle::Update()
{
	RigidBody::Update();
	wheelTransforms[0]->SetPosition(ToVec3(lastKnownWheelTransforms[0].p));
	wheelTransforms[0]->SetRotation(ToQuat(lastKnownWheelTransforms[0].q));
	wheelTransforms[1]->SetPosition(ToVec3(lastKnownWheelTransforms[1].p));
	wheelTransforms[1]->SetRotation(ToQuat(lastKnownWheelTransforms[1].q));
	wheelTransforms[2]->SetPosition(ToVec3(lastKnownWheelTransforms[2].p));
	wheelTransforms[2]->SetRotation(ToQuat(lastKnownWheelTransforms[2].q));
	wheelTransforms[3]->SetPosition(ToVec3(lastKnownWheelTransforms[3].p));
	wheelTransforms[3]->SetRotation(ToQuat(lastKnownWheelTransforms[3].q));

	if (KeyIsDown(SDLK_w))
		vehicleInputData.setDigitalAccel(true);
	else
		vehicleInputData.setDigitalAccel(false);

	if (KeyIsDown(SDLK_s))
		vehicleInputData.setDigitalBrake(true);
	else
		vehicleInputData.setDigitalBrake(false);

	if (KeyIsDown(SDLK_a))
		vehicleInputData.setDigitalSteerRight(true);
	else
		vehicleInputData.setDigitalSteerRight(false);

	if (KeyIsDown(SDLK_d))
		vehicleInputData.setDigitalSteerLeft(true);
	else
		vehicleInputData.setDigitalSteerLeft(false);

	if (KeyIsDown(SDLK_SPACE))
		vehicleInputData.setDigitalHandbrake(true);
	else
		vehicleInputData.setDigitalHandbrake(false);
}

void Vehicle::UpdateTransform()
{
	RigidBody::UpdateTransform();
	PxShape* shapes[4];
	rigidActor->getShapes(shapes, 4);
	lastKnownWheelTransforms[0] = shapes[0]->getLocalPose();
	lastKnownWheelTransforms[1] = shapes[1]->getLocalPose();
	lastKnownWheelTransforms[2] = shapes[2]->getLocalPose();
	lastKnownWheelTransforms[3] = shapes[3]->getLocalPose();
}

void Vehicle::PostProcessPhysics()
{
	
}



void Vehicle::BeforeVehicleUpdate(float deltaTime)
{
	static PxF32 steerVsForwardSpeedData[2 * 8] =
	{
		0.0f,		0.75f,
		5.0f,		0.75f,
		30.0f,		0.125f,
		120.0f,		0.1f,
		PX_MAX_F32, PX_MAX_F32,
		PX_MAX_F32, PX_MAX_F32,
		PX_MAX_F32, PX_MAX_F32,
		PX_MAX_F32, PX_MAX_F32
	};
	static PxFixedSizeLookupTable<8> steerVsForwardSpeedTable(steerVsForwardSpeedData, 4);
	static PxVehicleKeySmoothingData keySmoothingData =
	{
		{
			6.0f,	//rise rate eANALOG_INPUT_ACCEL
			6.0f,	//rise rate eANALOG_INPUT_BRAKE		
			6.0f,	//rise rate eANALOG_INPUT_HANDBRAKE	
			2.5f,	//rise rate eANALOG_INPUT_STEER_LEFT
			2.5f,	//rise rate eANALOG_INPUT_STEER_RIGHT
		},
		{
			10.0f,	//fall rate eANALOG_INPUT_ACCEL
			10.0f,	//fall rate eANALOG_INPUT_BRAKE		
			10.0f,	//fall rate eANALOG_INPUT_HANDBRAKE	
			5.0f,	//fall rate eANALOG_INPUT_STEER_LEFT
			5.0f	//fall rate eANALOG_INPUT_STEER_RIGHT
		}
	};

	PxVehicleDrive4WSmoothDigitalRawInputsAndSetAnalogInputs(keySmoothingData, steerVsForwardSpeedTable, vehicleInputData, deltaTime, airborne, *wheels);
}

void Vehicle::AfterVehicleUpdate(const physx::PxVehicleWheelQueryResult& queryResult)
{
	airborne = PxVehicleIsInAir(queryResult);
}
