#pragma once
#include "iRigidBody.h"
#include "iSoftBody.h"

namespace nPhysics
{
	class iPhysicsWorld
	{
	public:
		virtual ~iPhysicsWorld() {}

		virtual void TimeStep(float deltaTime) = 0;

		virtual void AddBody( iCollisionBody* collisionBody ) = 0;
		virtual void RemoveBody( iCollisionBody* collisionBody ) = 0;
		//virtual void AddRigidBody( iRigidBody* rigidBody ) = 0;
		//virtual void RemoveRigidBody( iRigidBody* rigidBody ) = 0;
	};
}