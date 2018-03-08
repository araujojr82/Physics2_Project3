#pragma once
//#include <Physics\Interfaces\iPhysicsWorld.h>
#include <iPhysicsWorld.h>
#include <vector>
#include "iCollisionBody.h"
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
		virtual bool Collide( iCollisionBody* bodyA, iCollisionBody* bodyB, float deltaTime );
		virtual bool Collide( cRigidBody* bodyA, cSoftBody* bodyB, float deltaTime );
		virtual bool Collide( cRigidBody* bodyA, cRigidBody* bodyB, float deltaTime );
		virtual sCollisionManifold FindCollision( iCollisionBody* bodyA, iCollisionBody* bodyB );

		virtual void SolveCollision( iCollisionBody* bodyA, iCollisionBody* bodyB, sCollisionManifold collisionResult, int c );
		virtual void SolveCollision( cRigidBody* bodyA, cSoftBody* bodyB, sCollisionManifold collisionResult, int c );

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

		//std::vector<iCollisionBody*> vecColliders1;
		//std::vector<iCollisionBody*> vecColliders2;

		//std::vector<iCollisionBody*> mCollisionBody;

		std::vector<iCollisionBody*> vecColliders1;
		std::vector<iCollisionBody*> vecColliders2;

		std::vector<iCollisionBody*> mCollisionBody;

		std::vector<cSoftBody*> mSoftyBody;
	};
}