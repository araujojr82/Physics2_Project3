#include "cBulletRigidBody.h"

namespace nPhysics
{
	cBulletRigidBody::cBulletRigidBody( const sRigidBodyDesc& desc, iShape* shape )
		: mShape( shape )
		, mPosition( desc.Position )
		, mPrevPosition( desc.PrevPosition )
		, mVelocity( desc.Velocity )
		, mMass( desc.Mass )
		, mInvMass( desc.invMass )
		, mRotation( desc.Rotation )
		, mAcceleration( desc.Acceleration )
		, mAngularVelocity( desc.AngularVelocity )
	{

		// Construct a Bullet body from the sRigidBodyDesc data
		
	
		if( shape->GetShapeType() == nPhysics::SHAPE_TYPE_PLANE )
		{
			float pConst;
			shape->GetPlaneConst( pConst );
			pConst = glm::normalize( pConst );
			glm::vec3 planeNormal;
			shape->GetPlaneNormal( planeNormal );

			btVector3 position = btVector3( desc.Position.x, desc.Position.y, desc.Position.z );

			// Had to invert the normals because I'm getting them from the mesh
			btVector3 pNormal = btVector3( planeNormal.x * -1, planeNormal.y * -1, planeNormal.z * -1 );						
			
			// Using plane Constant as 0 so the plane will be at the same position
			btCollisionShape* theShape = new btStaticPlaneShape( pNormal, 0 );
			

			btDefaultMotionState* motionState = new btDefaultMotionState( btTransform( btQuaternion( 0, 0, 0, 1 ), position ) );
			btRigidBody::btRigidBodyConstructionInfo
				rigidBodyCI( 0, motionState, theShape, btVector3( 0, 0, 0 ) );
	
			this->myBulletBody = new btRigidBody( rigidBodyCI );

			this->myBulletBody->setRestitution( 0.70 );

		}			
		else if( shape->GetShapeType() == nPhysics::SHAPE_TYPE_SPHERE )
		{
			btCollisionShape* theShape = nullptr;
			btVector3 position = btVector3( desc.Position.x, desc.Position.y, desc.Position.z );
			btDefaultMotionState* motionState = new btDefaultMotionState( btTransform( btQuaternion( 0, 0, 0, 1 ), position ) );

			float radius;
			shape->GetSphereRadius( radius );
			
			theShape = new btSphereShape( radius );

			btScalar mass = desc.Mass;
			btVector3 fallInertia( 0, 0, 0 );
			theShape->calculateLocalInertia( mass, fallInertia );

			btRigidBody::btRigidBodyConstructionInfo
				rigidBodyCI( mass, motionState, theShape, fallInertia );
			
			this->myBulletBody = new btRigidBody( rigidBodyCI );

			this->myBulletBody->setRestitution( 0.8 );
		}				

	}
	cBulletRigidBody::~cBulletRigidBody()
	{

	}

	iShape* cBulletRigidBody::GetShape()
	{
		return mShape;
	}

	void cBulletRigidBody::GetTransform(glm::mat4& transformOut)
	{
		btTransform trans;
		this->myBulletBody->getMotionState()->getWorldTransform( trans );
		btVector3 position = trans.getOrigin();		
		btQuaternion quatRotation = trans.getRotation();
		
		mRotation.x = quatRotation.getX();
		mRotation.y = quatRotation.getY();
		mRotation.z = quatRotation.getZ();
		mRotation.w = quatRotation.getW();

		transformOut = glm::mat4_cast( mRotation );
		transformOut[3][0] = position.getX();
		transformOut[3][1] = position.getY();
		transformOut[3][2] = position.getZ();
		transformOut[3][3] = 1.f;
	}

	void cBulletRigidBody::GetPosition(glm::vec3& positionOut)
	{
		//btTransform trans;
		//this->myBulletBody->getMotionState()->getWorldTransform( trans );
		//positionOut = glm::vec3( trans.getOrigin().getX(),
		//	trans.getOrigin().getY(),
		//	trans.getOrigin().getZ() );

		btVector3 position = this->myBulletBody->getCenterOfMassPosition();
		positionOut = glm::vec3( position.getX(),
								 position.getY(),
								 position.getZ() );
		

	
	}

	void cBulletRigidBody::GetVelocity( glm::vec3& velocityOut )
	{
		btVector3 btVelocity = this->myBulletBody->getLinearVelocity();
		velocityOut = glm::vec3( btVelocity.getX(), btVelocity.getY(), btVelocity.getZ() );
	}

	void cBulletRigidBody::SetVelocity( glm::vec3 velocityIn )
	{
		this->myBulletBody->activate( true );
		btVector3 btVelocity = btVector3( velocityIn.x, velocityIn.y, velocityIn.z );
		this->myBulletBody->setLinearVelocity( btVelocity );		

		return;
	}

	void cBulletRigidBody::GetRotation(glm::vec3& rotationOut)
	{
		btTransform trans;
		this->myBulletBody->getMotionState()->getWorldTransform( trans );
		btQuaternion quatRotation = trans.getRotation();

		mRotation.x = quatRotation.getX();
		mRotation.y = quatRotation.getY();
		mRotation.z = quatRotation.getZ();
		mRotation.w = quatRotation.getW();

		rotationOut = glm::eulerAngles(mRotation);
	}

	void cBulletRigidBody::GetRotation( glm::quat& rotationOut )
	{
		btTransform trans;
		this->myBulletBody->getMotionState()->getWorldTransform( trans );
		btQuaternion quatRotation = trans.getRotation();

		rotationOut.x = quatRotation.getX();
		rotationOut.y = quatRotation.getY();
		rotationOut.z = quatRotation.getZ();
		rotationOut.w = quatRotation.getW();
	}

	void cBulletRigidBody::SetRotation( glm::quat rotationIn )
	{

	}

	void cBulletRigidBody::SetTransform( glm::mat4 transformIn )
	{
		/*transformOut = glm::mat4_cast( mRotation );
		transformOut[3][0] = mPosition.x;
		transformOut[3][1] = mPosition.y;
		transformOut[3][2] = mPosition.z;
		transformOut[3][3] = 1.f;*/
	}

	void cBulletRigidBody::SetPosition( glm::vec3 positionIn )
	{
		btVector3 newPosition = btVector3( positionIn.x, positionIn.y, positionIn.z );

		btTransform trans;
		this->myBulletBody->getMotionState()->getWorldTransform( trans );
		trans.setOrigin( newPosition );
		this->myBulletBody->setWorldTransform( trans );

	}

	void cBulletRigidBody::SetRotation( glm::vec3 rotationIn )
	{
		mRotation = rotationIn;
		// glm::angleAxis( Euler angles )?
	}

	void cBulletRigidBody::ApplyForce( glm::vec3 force )
	{
		this->myBulletBody->activate( true );
		btVector3 btForce = btVector3( force.x, force.y, force.z );
		this->myBulletBody->applyCentralForce( btForce );
	}

	void cBulletRigidBody::ApplyForceAtPoint( glm::vec3 force, glm::vec3 velocity )
	{
		
	}


	void cBulletRigidBody::ApplyImpulse( glm::vec3 impulse )
	{		
		this->myBulletBody->activate( true );
		btVector3 btImpulse = btVector3( impulse.x, impulse.y, impulse.z );
		btImpulse *= this->mMass * 10;
		this->myBulletBody->applyCentralImpulse( btImpulse );
	}

	void cBulletRigidBody::ApplyImpulseAtPoint( glm::vec3 impulse, glm::vec3 relativePoint )
	{	
		this->myBulletBody->activate( true );
		btVector3 btImpulse = btVector3( impulse.x, impulse.y, impulse.z );
		btVector3 btPoint = btVector3( relativePoint.x, relativePoint.y, relativePoint.z );
		this->myBulletBody->applyImpulse( btImpulse, btPoint );
	}

	btRigidBody* cBulletRigidBody::GetBulletBody()
	{
		return this->myBulletBody;
	}
}