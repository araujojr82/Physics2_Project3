#include "cPhysicsFactory.h"
#include "cRigidBody.h"
#include "cSoftBody.h"
#include "cPhysicsWorld.h"
#include "shapes.h"

EXTERN_DLL_EXPORT nPhysics::iPhysicsFactory* CreateFactory()
{
	return new nPhysics::cPhysicsFactory();
}

namespace nPhysics
{
	cPhysicsFactory::~cPhysicsFactory() {}

	iPhysicsWorld* cPhysicsFactory::CreateWorld()
	{
		return new cPhysicsWorld();
	}

	iRigidBody* cPhysicsFactory::CreateRigidBody(const sRigidBodyDesc& desc, iShape* shape)
	{	
		return new cRigidBody(desc, shape);
		//return nullptr;
	}

	iSoftBody* cPhysicsFactory::CreateSoftBody( const sSoftBodyDesc& desc )
	{
		return new cSoftBody( desc );
		//return nullptr;
	}

	//iCollisionBody* cPhysicsFactory::CreateBody( const sBodyDesc& desc, eObjectType type )
	//{
	//	switch( type )
	//	{
	//		case nPhysics::RIGID_BODY:
	//		{
	//			iRigidBody* theRB = CreateRigidBody( desc, shape );
	//		}
	//			return 
	//			break;
	//		case nPhysics::SOFT_BODY:
	//			break;
	//		default:
	//			break;
	//	}
	//
	//	return nullptr;
	//}



	iShape* cPhysicsFactory::CreateSphere(float radius)
	{
		return new cSphereShape(radius);
	}
	iShape* cPhysicsFactory::CreatePlane(const glm::vec3& normal, float planeConst)
	{
		return new cPlaneShape(normal, planeConst);
	}
	
}