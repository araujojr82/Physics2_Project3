#include "cPhysicsBulletWorld.h"
#include <algorithm>

#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

const glm::vec3 GRAVITY = glm::vec3( 0.0f, 0.0f, -9.82f );

namespace nPhysics
{
	void cPhysicsBulletWorld::buildBulletWorld()
	{
		this->broadphase = new btDbvtBroadphase();
		this->collisionConfiguration = new btDefaultCollisionConfiguration();
		this->dispatcher = new btCollisionDispatcher( collisionConfiguration );
		this->solver = new btSequentialImpulseConstraintSolver;
		this->dynamicsWorld = new btDiscreteDynamicsWorld( dispatcher, broadphase, solver, collisionConfiguration );
		dynamicsWorld->setGravity( btVector3( 0, 0, -9.82 ) );

		return;
	}	

	cPhysicsBulletWorld::~cPhysicsBulletWorld()
	{
		return;
	}

	void cPhysicsBulletWorld::TimeStep( float deltaTime )
	{			
		btScalar btdeltaTime = deltaTime;
		dynamicsWorld->stepSimulation( btdeltaTime );
		return;
	}

	void cPhysicsBulletWorld::AddRigidBody( nPhysics::iRigidBody* rigidBody )
	{
		cBulletRigidBody* rb = dynamic_cast< cBulletRigidBody* >( rigidBody );
		
		this->dynamicsWorld->addRigidBody( rb->GetBulletBody() );

		return;
	}

	void cPhysicsBulletWorld::RemoveRigidBody( nPhysics::iRigidBody* rigidBody )
	{
		cBulletRigidBody* rb = dynamic_cast< cBulletRigidBody* >( rigidBody );

		this->dynamicsWorld->removeRigidBody( rb->GetBulletBody() );

		return;
	}

}