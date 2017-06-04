#pragma once
#include <vector>
#include <btBulletDynamicsCommon.h>
#include "Singleton.h"

class RigidBody;

class Physics : public Singleton<Physics>
{
public:
	Physics();
	~Physics();

	void RegisterRigidBody(RigidBody* rigidBody);
	void UnregisterRigidBody(RigidBody* rigidBody);
	void Step(float deltaTime);
	void DebugDraw();

	bool Visualize() const { return visualize; }
	void SetVisualize(bool visualize) { this->visualize = visualize; }

private:
	
	btCollisionConfiguration* config;
	btDispatcher* dispatcher;
	btBroadphaseInterface* broadphase;
	btConstraintSolver* constraintSolver;
	btDynamicsWorld* world;
	bool visualize{};

	std::vector<RigidBody*> rigidBodies;
	static void PostProcessPhysicsSubtick(btDynamicsWorld *world, btScalar timeStep);
};

