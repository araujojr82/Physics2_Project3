#pragma once
#include "iRigidBody.h"
#include "iSoftBody.h"
#include "sBodyDesc.h"
#include "sRigidBodyDesc.h"
#include "sSoftBodyDesc.h"
#include "iShape.h"
#include "iPhysicsWorld.h"

namespace nPhysics
{
	class iPhysicsFactory
	{
	public:
		virtual ~iPhysicsFactory() {}

		virtual iPhysicsWorld* CreateWorld() = 0;

		virtual iRigidBody* CreateRigidBody( const sRigidBodyDesc& desc, iShape* shape ) = 0;
		virtual iSoftBody* CreateSoftBody( const sSoftBodyDesc& desc ) = 0;
		//virtual iCollisionBody* CreateBody( const sBodyDesc& desc, eObjectType type ) = 0;
		
		virtual iShape* CreateSphere(float radius) = 0;
		virtual iShape* CreatePlane(const glm::vec3& normal, float planeConst) = 0;
	};
}