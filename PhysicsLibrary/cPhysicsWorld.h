#pragma once
//#include <Physics\Interfaces\iPhysicsWorld.h>
#include <iPhysicsWorld.h>
#include <vector>
#include "cCollisionBody.h"
#include "cRigidBody.h"
#include "cSoftBody.h"
#include "shapes.h"
#include "cIntegration.h"
#include "CollisionManifold.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp> // <- This includes vec3 and mat4

namespace nPhysics
{
	class cPhysicsWorld : public iPhysicsWorld
	{
	public:
		virtual ~cPhysicsWorld();

		virtual void TimeStep(float deltaTime);

		virtual void AddBody( iCollisionBody* collisionBody );
		virtual void RemoveBody( iCollisionBody* collisionBody );
		virtual bool Collide( cCollisionBody* bodyA, cCollisionBody* bodyB, float deltaTime );
		virtual sCollisionManifold FindCollision( cCollisionBody* bodyA, cCollisionBody* bodyB );

		//virtual void AddRigidBody(iRigidBody* rigidBody);
		//virtual void RemoveRigidBody(iRigidBody* rigidBody);
		//virtual bool Collide( cRigidBody* bodyA, cRigidBody* bodyB, float deltaTime );
		//virtual sCollisionManifold FindCollision( cRigidBody* bodyA, cRigidBody* bodyB );
		virtual sCollisionManifold FindColSphereSphere( cRigidBody* bodyA, cSphereShape* sphereA, cRigidBody* bodyB, cSphereShape* sphereB );
		virtual sCollisionManifold FindColSpherePlane( cRigidBody* bodyA, cSphereShape* sphereA, cRigidBody* bodyB, cPlaneShape* planeB );
		virtual void SolveCollision( cRigidBody* m1, cRigidBody* m2, sCollisionManifold collisionResult, int j );

		virtual bool CollideSpherePlane( cRigidBody* bodyA, cSphereShape* sphereA, cRigidBody* bodyB, cPlaneShape* planeB );
		virtual bool CollideSphereSphere( cRigidBody* bodyA, cSphereShape* sphereA, cRigidBody* bodyB, cSphereShape* sphereB );

		cIntegration myIntegrator;

	private:

		std::vector<sCollisionManifold> collisionResults;

		//std::vector<cRigidBody*> vecColliders1;
		//std::vector<cRigidBody*> vecColliders2;

		//std::vector<cRigidBody*> mRigidBody;

		std::vector<cCollisionBody*> vecColliders1;
		std::vector<cCollisionBody*> vecColliders2;

		std::vector<cCollisionBody*> mRigidBody;
	};
}