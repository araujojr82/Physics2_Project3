#pragma once
#include <iRigidBody.h>
#include <sRigidBodyDesc.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp> // <- This includes vec3 and mat4

#include <btBulletDynamicsCommon.h>

namespace nPhysics
{
	class cPhysicsWorld;
	class cBulletRigidBody : public iRigidBody
	{
	public:
		cBulletRigidBody(const sRigidBodyDesc& desc, iShape* shape);
		virtual ~cBulletRigidBody();

		virtual iShape* GetShape();

		virtual void GetTransform(glm::mat4& transformOut);
		virtual void GetPosition(glm::vec3& positionOut);
		virtual void GetVelocity( glm::vec3& velocityOut );
		virtual void GetRotation(glm::vec3& rotationOut);
		virtual void GetRotation( glm::quat& rotationOut );		

		virtual void SetTransform( glm::mat4 transformIn );
		virtual void SetPosition( glm::vec3 positionIn );
		virtual void SetVelocity( glm::vec3 velocityIn );
		virtual void SetRotation( glm::vec3 rotationIn );
		virtual void SetRotation( glm::quat rotationIn );

		virtual void ApplyForce( glm::vec3 force );
		virtual void ApplyForceAtPoint( glm::vec3 force, glm::vec3 velocity );

		virtual void ApplyImpulse( glm::vec3 impulse );
		virtual void ApplyImpulseAtPoint( glm::vec3 impulse, glm::vec3 relativePoint );

		btRigidBody* GetBulletBody();

	private:
		friend class cPhysicsWorld;
		iShape* mShape;
		glm::vec3 mPosition;
		glm::vec3 mPrevPosition;
		glm::vec3 mVelocity;
		glm::quat mRotation;

		glm::vec3 mAcceleration;
		glm::vec3 mAngularVelocity;

		float mMass;
		float mInvMass;

		btRigidBody* myBulletBody;
	};
}