#pragma once
#include <iPhysicsFactory.h>

#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)

EXTERN_DLL_EXPORT nPhysics::iPhysicsFactory* CreateFactory();

namespace nPhysics
{
	class cPhysicsBulletFactory : public iPhysicsFactory
	{
	public:
		virtual ~cPhysicsBulletFactory();

		virtual iPhysicsWorld* CreateWorld();

		virtual iRigidBody* CreateRigidBody(const sRigidBodyDesc& desc, iShape* shape);

		virtual iShape* CreateSphere(float radius);
		virtual iShape* CreatePlane(const glm::vec3& normal, float planeConst);
	};
}