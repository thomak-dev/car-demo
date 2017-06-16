#include "Vehicle.h"
#include <vehicle/PxVehicleSDK.h>
#include <vehicle/PxVehicleUtil.h>
#include "input.h"
#include "Entity.h"
#include "Transform.h"
#include "math_utility.h"
#include "Physics.h"
#include "Wheel.h"

using namespace physx;

static std::unordered_map<std::string, Tire::Type> stringToType
{
	{ "Normal", Tire::Type::Normal },
	{ "Slick", Tire::Type::Slick },
	{ "Winter", Tire::Type::Winter },
	{ "Spikes", Tire::Type::Spikes }
};

Tire::Type Tire::FromString(const std::string& str)
{
	return stringToType[str];
}

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
	if (json.HasMember("steer"))
		steer = glm::radians(json["steer"].GetFloat());
	if (json.HasMember("tire_type"))
		tireType = Tire::FromString(json["tire_type"].GetString());
}

PxConvexMesh* Vehicle::CreateWheelMesh(float radius, float width)
{
	const int VertexCount = 64;
	PxVec3 verts[VertexCount];
	const int PolyCount = 34; // 32 circle points + lids
	PxHullPolygon polys[PolyCount];
	const int IndexCount = 192; // 4 verts * 32 faces + 32 verts * 2 faces (lids)
	uint32_t indices[IndexCount];

	const int NumCirclePoints = 32;
	const float xLeft = width / 2; // x coord of left lid
	const float xRight = -xLeft; // x coord of right lid

	float angle = 0;
	for (int i = 0; i < NumCirclePoints; ++i)
	{
		float z = cos(angle) * radius;
		float y = sin(angle) * radius;

		PxVec3 normal{ 0, sin(angle + Float::Pi / NumCirclePoints), cos(angle + Float::Pi / NumCirclePoints) };
		polys[i].mPlane[0] = normal.x;
		polys[i].mPlane[1] = normal.y;
		polys[i].mPlane[2] = normal.z;
		polys[i].mPlane[3] = -(normal.y * y + normal.z * z);
		polys[i].mNbVerts = 4;
		polys[i].mIndexBase = i * 4;
		verts[i * 2].x = xLeft;
		verts[i * 2].y = y;
		verts[i * 2].z = z;
		verts[i * 2 + 1].x = xRight;
		verts[i * 2 + 1].y = y;
		verts[i * 2 + 1].z = z;
		uint32_t index = i * 2;			

		indices[i * 4] = (index + 2) % (NumCirclePoints * 2);
		indices[i * 4 + 1] = (index + 3) % (NumCirclePoints * 2);
		indices[i * 4 + 2] = index + 1;
		indices[i * 4 + 3] = index;

		angle += 2 * Float::Pi / NumCirclePoints;
	}
	for (int i = 0; i < NumCirclePoints; ++i)
		indices[NumCirclePoints * 4 + i] = (NumCirclePoints - i - 1) * 2;
	for (int i = 0; i < NumCirclePoints; ++i)
		indices[NumCirclePoints * 5 + i] = i * 2 + 1;

	polys[NumCirclePoints].mPlane[0] = 1;
	polys[NumCirclePoints].mPlane[1] = 0;
	polys[NumCirclePoints].mPlane[2] = 0;
	polys[NumCirclePoints].mPlane[3] = -xLeft;
	polys[NumCirclePoints].mNbVerts = 32;
	polys[NumCirclePoints].mIndexBase = IndexCount - 2 * NumCirclePoints;

	polys[NumCirclePoints + 1].mPlane[0] = -1;
	polys[NumCirclePoints + 1].mPlane[1] = 0;
	polys[NumCirclePoints + 1].mPlane[2] = 0;
	polys[NumCirclePoints + 1].mPlane[3] = xRight;
	polys[NumCirclePoints + 1].mNbVerts = 32;
	polys[NumCirclePoints + 1].mIndexBase = IndexCount - NumCirclePoints;

	PxConvexMeshDesc convexMeshDesc;
	convexMeshDesc.points.count = VertexCount;
	convexMeshDesc.points.data = verts;
	convexMeshDesc.points.stride = sizeof(PxVec3);
	convexMeshDesc.polygons.count = PolyCount;
	convexMeshDesc.polygons.data = polys;
	convexMeshDesc.polygons.stride = sizeof(PxHullPolygon);
	convexMeshDesc.indices.count = IndexCount;
	convexMeshDesc.indices.data = indices;
	convexMeshDesc.indices.stride = sizeof(uint32_t);

#ifdef _DEBUG
	// mesh should be validated before cooking without the mesh cleaning
	bool res = physics->cooking->validateConvexMesh(convexMeshDesc);
	SDL_assert(res);
#endif
	PxConvexMesh* mesh = physics->cooking->createConvexMesh(convexMeshDesc, physics->backend->getPhysicsInsertionCallback());
	return mesh;
}

void Vehicle::Initialize()
{
	Component::Initialize();
	const PxU32 numWheels = 4;

	transform = entity.GetComponent<Transform>();
	PxTransform pTrans{ ToPxMat44(transform->WorldMatrix()) };
	PxRigidDynamic* dynamicActor = physics->backend->createRigidDynamic(pTrans.getNormalized());
	rigidActor = dynamicActor;

	PxVec3 wheelOffsets[numWheels];
	std::vector<Wheel*> wheelComponents;
	entity.GetComponentsInDescendants<Wheel>(std::back_inserter(wheelComponents));
	SDL_assert(wheelComponents.size() >= 4);
	std::sort(wheelComponents.begin(), wheelComponents.end(), [](Wheel* a, Wheel* b) { return a->GetIndex() < b->GetIndex(); });
	for (int i = 0; i < numWheels; ++i)
	{
		wheelTransforms.push_back(wheelComponents[i]->GetEntity().GetComponent<Transform>());
		PxTransform globalPose{ ToPxMat44(wheelTransforms[i]->WorldMatrix()) };
		lastKnownWheelTransforms.push_back(globalPose.getNormalized());
		wheelOffsets[i] = ToPxVec3(wheelTransforms[i]->Position());
	}

	PxFilterData wheelFilterData;
	physics->SetUpFilterData(wheelFilterData, EntityFlags::Wheel);

	PxConvexMesh* convexMesh = CreateWheelMesh(wheelRadius, wheelWidth);

	for (PxU32 i = 0; i < numWheels; i++)
	{
		PxConvexMeshGeometry geom{ convexMesh };		
		PxShape* wheelShape = PxRigidActorExt::createExclusiveShape(*dynamicActor, geom, *material);
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
	qryFilterData.word1 = EntityFlags::Ground;

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

		tires[i].mType = tireType;
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
	engine.mMaxOmega = 300.0f;
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
	ackermann.mFrontWidth =	wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_RIGHT).x - wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_LEFT).x;
	ackermann.mRearWidth = wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_RIGHT).x - wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_LEFT).x;

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
