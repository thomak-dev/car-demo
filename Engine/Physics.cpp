#include "Physics.h"
#include <algorithm>
#include "RigidBody.h"
#include "Renderer.h"

Physics::Physics()
{	
	config = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(config);
	broadphase = new btDbvtBroadphase();
	constraintSolver = new btSequentialImpulseConstraintSolver();
	world = new btDiscreteDynamicsWorld(dispatcher, broadphase, constraintSolver, config);

	world->setGravity(btVector3(0, -9.81f, 0));
	world->setDebugDrawer(Renderer::Instance().GetPhysicsDebugDrawer());
}


Physics::~Physics()
{
	delete world;
	delete constraintSolver;
	delete broadphase;
	delete dispatcher;
	delete config;
}

void Physics::RegisterRigidBody(RigidBody* rigidBody)
{
	rigidBodies.push_back(rigidBody);
	world->addRigidBody(rigidBody->bulletRigidBody);
}

void Physics::UnregisterRigidBody(RigidBody* rigidBody)
{
	auto found = std::find(rigidBodies.begin(), rigidBodies.end(), rigidBody);
	if(found != rigidBodies.end())
	{
		rigidBodies.erase(found);
		world->removeRigidBody(rigidBody->bulletRigidBody);
	}
}

void Physics::Step(float deltaTime)
{
	world->stepSimulation(deltaTime);
}

void Physics::DebugDraw()
{
	world->debugDrawWorld();
}
