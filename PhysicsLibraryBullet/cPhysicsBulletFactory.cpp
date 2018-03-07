#include "cPhysicsBulletFactory.h"
#include "cBulletRigidBody.h"
#include "cPhysicsBulletWorld.h"
#include "shapes.h"

EXTERN_DLL_EXPORT nPhysics::iPhysicsFactory* CreateFactory()
{
	return new nPhysics::cPhysicsBulletFactory();
}

namespace nPhysics
{	
	cPhysicsBulletFactory::~cPhysicsBulletFactory() {}

	iPhysicsWorld* cPhysicsBulletFactory::CreateWorld()
	{	
		cPhysicsBulletWorld* thePhysicsWorld = new cPhysicsBulletWorld();

		thePhysicsWorld->buildBulletWorld();

		return thePhysicsWorld;
	}

	iRigidBody* cPhysicsBulletFactory::CreateRigidBody(const sRigidBodyDesc& desc, iShape* shape)
	{
		return new cBulletRigidBody(desc, shape);
	}

	iShape* cPhysicsBulletFactory::CreateSphere(float radius)
	{
		return new cSphereShape(radius);
	}
	iShape* cPhysicsBulletFactory::CreatePlane(const glm::vec3& normal, float planeConst)
	{
		return new cPlaneShape(normal, planeConst);
	}
	
}