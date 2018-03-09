#include "cPhysicsWorld.h"
#include <algorithm>

#include "CollisionDetection.h"

#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

const glm::vec3 GRAVITY = glm::vec3( 0.0f, 0.0f, -9.82f );

namespace nPhysics
{
	cPhysicsWorld::~cPhysicsWorld()
	{

	}

	void cPhysicsWorld::AddBody( iCollisionBody* collisionBody )
	{	
		eObjectType bodyType = collisionBody->getType();		

		if( bodyType == RIGID_BODY )
		{
			std::vector<iCollisionBody*>::iterator itRigidBody;
			itRigidBody = std::find( mCollisionBody.begin(), mCollisionBody.end(), collisionBody );
			if( itRigidBody == mCollisionBody.end() )
			{
				mCollisionBody.push_back( collisionBody );
			}
		}

		else
		{
			cSoftBody* softBody = dynamic_cast< cSoftBody* > ( collisionBody );
			// A soft body is made of rigid body nodes, so we need to add them
			
			std::vector<iRigidBody*> theSBNodes = softBody->getNodeListAsRigidBodies();

			for( int i = 0; i != theSBNodes.size(); i++ )
			{
				mCollisionBody.push_back( theSBNodes[i] );
			}

			mSoftyBody.push_back( softBody );
		}
		
	}

	void cPhysicsWorld::RemoveBody( iCollisionBody* collisionBody )
	{
		std::vector<iCollisionBody*>::iterator itRigidBody;

		itRigidBody = std::remove( mCollisionBody.begin(), mCollisionBody.end(), collisionBody );
		if( itRigidBody != mCollisionBody.end() )
		{
			mCollisionBody.resize( mCollisionBody.size() - 1 );
		}
	}

	void cPhysicsWorld::TimeStep( float deltaTime )
	{
		collisionResults.clear();
		vecColliders1.clear();
		vecColliders2.clear();	

		int numBodies = mCollisionBody.size();

		for( int i = 0; i != numBodies; i++ )
		{
			for( int j = 0; j != numBodies; j++ )
			{
				if( i == j ) continue; //Don't test for itself

				sCollisionManifold result;
				result = FindCollision( mCollisionBody[i], mCollisionBody[j] );

				if( result.colliding )
				{
					vecColliders1.push_back( mCollisionBody[i] );
					vecColliders2.push_back( mCollisionBody[j] );
					collisionResults.push_back( result );
				}
			}
		}

		//glm::vec3 force = GRAVITY;

		// Apply forces
		for( int i = 0; i != numBodies; i++ )
		{
			eObjectType bodyTypeA = mCollisionBody[i]->getType();

			if( bodyTypeA == RIGID_BODY )
			{
				cRigidBody* theBody = dynamic_cast< cRigidBody* > ( mCollisionBody[i] );

				if( theBody->bIsStatic ) continue;

				eShapeType type = theBody->GetShape()->GetShapeType();
				if( type != nPhysics::SHAPE_TYPE_PLANE )
					theBody->ApplyForce( GRAVITY );
			}
			//if( bodyTypeA == SOFT_BODY )
			//{
			//	cSoftBody* theBody = dynamic_cast< cSoftBody* > ( mCollisionBody[i] );

			//	glm::vec3 force = GRAVITY;
			//	//theBody->ApplyForce( GRAVITY );
			//}
		}

		//// Apply force on soft bodies too
		//for( int i = 0; i != mSoftyBody.size(); ++i )
		//{
		//	mSoftyBody[i]->ApplyForce( GRAVITY );
		//}

		// Apply impulses to resolve collisions
		for( int k = 0; k < 10; k++ ) 
		{ 
			for( int i = 0; i != collisionResults.size(); i++ ) 
			{
				for( int j = 0; j != collisionResults[i].contacts.size(); j++ ) 
				{
					SolveCollision( vecColliders1[i], vecColliders2[i], collisionResults[i], j );
				}
			}
		}

		// Integrate velocity and impulse of objects
		for( int i = 0; i != numBodies; i++ )
		{
			eObjectType bodyTypeA = mCollisionBody[i]->getType();
			cRigidBody* theBody = NULL;

			if( bodyTypeA == RIGID_BODY )
			{
				theBody = dynamic_cast< cRigidBody* > ( mCollisionBody[i] );

				if( theBody->bIsStatic ) continue;

				//cRigidBody* theBody = mRigidBody[i];
				glm::vec3 force = theBody->mAcceleration;
				glm::vec3 acceleration = force * theBody->mInvMass;

				eShapeType type = theBody->GetShape()->GetShapeType();
				if( type != nPhysics::SHAPE_TYPE_PLANE )

				if( theBody->isParticle() )
					this->myIntegrator.Verlet( theBody->mPosition, theBody->mPrevPosition, acceleration, deltaTime );
				else
					this->myIntegrator.RK4( theBody->mPosition, theBody->mVelocity, acceleration, deltaTime );
					//this->myIntegrator.Euler( mRigidBody[i]->mPosition, mRigidBody[i]->mVelocity, acceleration, deltaTime );
			}
		}

		//// Integrate soft bodies too
		//for( int i = 0; i != mSoftyBody.size(); ++i )
		//{
		//	std::vector<iRigidBody*> theNodes = mSoftyBody[i]->getNodeListAsRigidBodies();

		//	for( int j = 0; j != theNodes.size(); j++ )
		//	{
		//		// Cast as cRigidBody
		//		cRigidBody *theBody = dynamic_cast< cRigidBody* > ( theNodes[j] );
		//		
		//		if( !theBody->bIsStatic )
		//		{
		//			glm::vec3 force = theBody->mAcceleration;
		//			glm::vec3 acceleration = force * theBody->mInvMass;
		//			glm::vec3 position;
		//			glm::vec3 velocity;

		//			theBody->GetPosition( position );
		//			theBody->GetVelocity( velocity );

		//			this->myIntegrator.RK4( position, velocity, acceleration, deltaTime );
		//			//this->myIntegrator.Euler( position, velocity, acceleration, deltaTime );

		//			theBody->SetPosition( position );
		//			theBody->SetVelocity( velocity );
		//		}
		//			
		//	}
		//}

		// Correct position to avoid sinking!
		for( int i = 0; i != collisionResults.size(); i++ )
		{
			eObjectType bodyTypeA = vecColliders1[i]->getType();
			eObjectType bodyTypeB = vecColliders2[i]->getType();

			cRigidBody* m1 = NULL;
			cRigidBody* m2 = NULL;

			if( bodyTypeA == RIGID_BODY )
				m1 = dynamic_cast< cRigidBody* > ( vecColliders1[i] );

			if( bodyTypeB == RIGID_BODY )
				m2 = dynamic_cast< cRigidBody* > ( vecColliders2[i] );

			if( bodyTypeA == RIGID_BODY && bodyTypeB == RIGID_BODY )
			{
				//cRigidBody* m1 = vecColliders1[i];
				//cRigidBody* m2 = vecColliders2[i];
				float totalMass = m1->mInvMass + m2->mInvMass;

				eShapeType type1 = m1->GetShape()->GetShapeType();
				eShapeType type2 = m2->GetShape()->GetShapeType();

				float LinearProjectionPercent = 0.8f;
				float PenetrationSlack = 0.01f;

				float depth = fmaxf( collisionResults[i].depth - PenetrationSlack, 0.0f );
				float scalar = ( totalMass == 0.0f ) ? 0.0f : depth / totalMass;
				glm::vec3 correction = collisionResults[i].normal * scalar * LinearProjectionPercent;

				if( !m1->bIsStatic )
					m1->mPosition = m1->mPosition - correction * m1->mInvMass;
				if( !m2->bIsStatic )
					m2->mPosition = m2->mPosition + correction * m2->mInvMass;
			}
		}

		//Apply spring forces for softbodies
		for( int i = 0; i != mSoftyBody.size(); ++i )
		{
			mSoftyBody[i]->ApplySpringForces();
		}

		//// Same as above, solve soft constraints
		//for( int i = 0, size = mSoftyBody.size(); i < size; ++i )
		//{
		//	mSoftyBody[i]->SolveConstraints( constraints );
		//}


		int x = 0;
		// Clear acceleration
		for( int i = 0; i != numBodies; i++ ) 
		{
			eObjectType bodyTypeA = mCollisionBody[i]->getType();
			cRigidBody* theBody = NULL;

			if( bodyTypeA == RIGID_BODY )
			{
				theBody = dynamic_cast< cRigidBody* > ( mCollisionBody[i] );
				//cRigidBody* theBody = mRigidBody[i];
				theBody->mAcceleration = glm::vec3( 0.0f );
			}
			x++;
		}

		//// Clear soft bodies too
		//for( int i = 0; i != mSoftyBody.size(); ++i )
		//{
		//	std::vector<iRigidBody*> theNodes = mSoftyBody[i]->getNodeListAsRigidBodies();

		//	for( int j = 0; j != theNodes.size(); j++ )
		//	{
		//		// Cast as cRigidBody
		//		cRigidBody *theBody = dynamic_cast< cRigidBody* > ( theNodes[j] );

		//		theBody->mAcceleration = glm::vec3( 0.0f );
		//	}
		//}

	}

	//sCollisionManifold cPhysicsWorld::FindCollision( cRigidBody* bodyA, cRigidBody* bodyB )
	sCollisionManifold cPhysicsWorld::FindCollision( iCollisionBody* bodyA, iCollisionBody* bodyB )
	{
		sCollisionManifold result;

		eObjectType bodyTypeA = bodyA->getType();
		eObjectType bodyTypeB = bodyB->getType();

		cRigidBody* rbA = NULL;
		cRigidBody* rbB = NULL;
		cSoftBody* sbA = NULL;
		cSoftBody* sbB = NULL;

		if( bodyTypeA == RIGID_BODY )
			rbA = dynamic_cast< cRigidBody* > ( bodyA );
		else if( bodyTypeA == SOFT_BODY )
			sbA = dynamic_cast< cSoftBody* > ( bodyA );

		if( bodyTypeB == RIGID_BODY )
			rbB = dynamic_cast< cRigidBody* > ( bodyB );
		else if( bodyTypeB == SOFT_BODY )
			sbB = dynamic_cast< cSoftBody* > ( bodyB );

		if( bodyTypeA == RIGID_BODY && bodyTypeB == RIGID_BODY )
		{
			eShapeType typeA = rbA->GetShape()->GetShapeType();
			eShapeType typeB = rbB->GetShape()->GetShapeType();

			if( typeA == nPhysics::SHAPE_TYPE_SPHERE )
			{
				cSphereShape * sphereA = dynamic_cast< cSphereShape* > ( rbA->GetShape() );

				if( typeB == nPhysics::SHAPE_TYPE_SPHERE )
				{
					cSphereShape * sphereB = dynamic_cast< cSphereShape* > ( rbB->GetShape() );

					if( rbA->isParticle() && rbB->isParticle() )
					{
						// Don't collide particles
					}

					else if( rbA->isParticle() || rbB->isParticle() )
					{
						glm::vec3 pA;
						rbA->GetPosition( pA );
						float radiusA;
						sphereA->GetSphereRadius( radiusA );

						glm::vec3 pB;
						rbB->GetPosition( pB );
						float radiusB;
						sphereB->GetSphereRadius( radiusB );

						if( rbA->isParticle() )
						{
							glm::vec3 v = pA - pB;
							float length = glm::length( v );
							if( length < radiusB ) // if the particle is inside the ball
							{
								// project the particle to the surface of the ball
								pA += glm::normalize( v ) * ( radiusB - length );
								rbA->SetPosition( pA );
							}
						}
						else
						{
							glm::vec3 v = pB - pA;
							float length = glm::length( v );
							if( length < radiusA ) // if the particle is inside the ball
							{
								// project the particle to the surface of the ball
								pB += glm::normalize( v ) * ( radiusA - length );
								rbB->SetPosition( pB );
							}
						}
					}

					else
					{
						result = FindColSphereSphere( rbA, sphereA, rbB, sphereB );
					}
				}
				else if( typeB == nPhysics::SHAPE_TYPE_PLANE )
				{
					cPlaneShape* planeB = dynamic_cast< cPlaneShape* >( rbB->GetShape() );

					result = FindColSpherePlane( rbA, sphereA, rbB, planeB );
					result.normal = result.normal * -1.0f;
				}
			}
			else if( typeA == nPhysics::SHAPE_TYPE_PLANE )
			{
				cPlaneShape* planeA = dynamic_cast< cPlaneShape* >( rbA->GetShape() );
				if( typeB == nPhysics::SHAPE_TYPE_PLANE )
				{
					// Do Nothing
				}
				else if( typeB == nPhysics::SHAPE_TYPE_SPHERE )
				{
					cSphereShape * sphereB = dynamic_cast< cSphereShape* > ( rbB->GetShape() );
					result = FindColSpherePlane( rbB, sphereB, rbA, planeA );
				}
			}
		}

		return result;
	}

	sCollisionManifold cPhysicsWorld::FindColSphereSphere( cRigidBody* bodyA, cSphereShape* sphereA, cRigidBody* bodyB, cSphereShape* sphereB )
	{
		sCollisionManifold result;

		float radA, radB;
		sphereA->GetSphereRadius( radA );
		sphereB->GetSphereRadius( radB );

		float radii = radA + radB;
		glm::vec3 distance = bodyB->mPosition - bodyA->mPosition;

		if( glm::length2( distance ) - radii * radii > 0 || glm::length2( distance ) == 0.0f ) {
			return result;
		}
		glm::normalize( distance );
		
		result.colliding = true;
		result.normal = distance;
		result.depth = fabsf( glm::length( distance ) - radii ) * 0.5f;

		// Distance to intersection point
		float dtp = radA - result.depth;
		glm::vec3 contact = bodyA->mPosition + distance * dtp;

		result.contacts.push_back( contact );

		return result;
	}

	glm::vec3 ClosestPoint( glm::vec3 planeNormal, float planeConst, glm::vec3 thePoint ) 
	{
		float distance = glm::dot( planeNormal, thePoint ) - planeConst;	
		return thePoint - planeNormal * distance;
	}

	bool CMP( float x, float y )
	{
		return ( fabsf( x - y ) <= FLT_EPSILON * fmaxf( 1.0f, fmaxf( fabsf( x ), fabsf( y ) ) ) );
	}

	sCollisionManifold cPhysicsWorld::FindColSpherePlane( cRigidBody* bodyA, cSphereShape* sphereA, cRigidBody* bodyB, cPlaneShape* planeB )
	{
		sCollisionManifold result;

		float radA;
		sphereA->GetSphereRadius( radA );

		glm::vec3 pNormal;
		planeB->GetPlaneNormal( pNormal );
		float pConst;
		planeB->GetPlaneConst( pConst );

		glm::vec3 closestPoint = ClosestPoint( pNormal, pConst, bodyA->mPosition );

		float distanceSq = glm::length2( closestPoint - bodyA->mPosition );
		if( distanceSq > radA * radA ) {
			return result;
		}

		glm::vec3 normal;
		if( CMP( distanceSq, 0.0f ) ) {
			if( CMP( glm::length2( closestPoint - bodyB->mPosition ), 0.0f ) ) {
				return result;

			}
			// Closest point is at the center of the sphere
			normal = glm::normalize( closestPoint - bodyB->mPosition );
		}
		else {
			normal = glm::normalize( bodyA->mPosition - closestPoint );
		}

		glm::vec3 outsidePoint = bodyA->mPosition - normal * radA;

		float distance = glm::length( closestPoint - outsidePoint );

		result.colliding = true;
		result.contacts.push_back( closestPoint + ( outsidePoint - closestPoint ) * 0.5f );
		result.normal = normal;
		result.depth = distance * 0.5f;
		//result.depth = distance;

		return result;
	}

	void cPhysicsWorld::SolveCollision( iCollisionBody* bodyA, iCollisionBody* bodyB, sCollisionManifold collisionResult, int c )
	{
		eObjectType bodyTypeA = bodyA->getType();
		eObjectType bodyTypeB = bodyB->getType();

		cRigidBody* rbA = NULL;
		cRigidBody* rbB = NULL;
		cSoftBody* sbA = NULL;
		cSoftBody* sbB = NULL;

		if( bodyTypeA == RIGID_BODY )
			rbA = dynamic_cast< cRigidBody* > ( bodyA );
		else if( bodyTypeA == SOFT_BODY )
			sbA = dynamic_cast< cSoftBody* > ( bodyA );

		if( bodyTypeB == RIGID_BODY )
			rbB = dynamic_cast< cRigidBody* > ( bodyB );
		else if( bodyTypeB == SOFT_BODY )
			sbB = dynamic_cast< cSoftBody* > ( bodyB );

		// Both are rigid bodies, use the good old Collide
		if( bodyTypeA == RIGID_BODY && bodyTypeB == RIGID_BODY )
		{
			SolveCollision( rbA, rbB, collisionResult, c );
		}
		else
		{
			// Both are soft bodies, I'm not there yet
			if( bodyTypeA == SOFT_BODY && bodyTypeB == SOFT_BODY )
			{
				// Do nothing
			}
			// A is a rigid body, while B isn't
			else if( bodyTypeA == RIGID_BODY && bodyTypeB == SOFT_BODY )
			{
				SolveCollision( rbA, sbB, collisionResult, c );
			}
			else		// A must be soft them...
			{
				SolveCollision( rbB, sbA, collisionResult, c );
			}

		}
	}
	
	void cPhysicsWorld::SolveCollision( cRigidBody* bodyA, cSoftBody* bodyB, sCollisionManifold collisionResult, int c )
	{

	}

	void cPhysicsWorld::SolveCollision( cRigidBody* bodyA, cRigidBody* bodyB, sCollisionManifold collisionResult, int c )
	{

		float invMass1 = bodyA->mInvMass;
		float invMass2 = bodyB->mInvMass;
		float invMassSum = invMass1 + invMass2;

		if( invMassSum == 0.0f ) {
			return; // Both objects have infinate mass!
		}

		glm::vec3 r1 = collisionResult.contacts[c] - bodyA->mPosition;
		glm::vec3 r2 = collisionResult.contacts[c] - bodyB->mPosition;

		// Relative velocity
		glm::vec3 relativeVel = bodyB->mVelocity - bodyA->mVelocity;

		// Relative collision normal
		glm::vec3 relativeNorm = collisionResult.normal;
		glm::normalize( relativeNorm );

		// Moving away from each other? Do nothing!
		if( glm::dot( relativeVel, relativeNorm ) > 0.0f ) {
			return;
		}

		float e = 0.5f; // fminf( A.cor, B.cor );

		float numerator = ( -( 1.0f + e ) * glm::dot( relativeVel, relativeNorm ) );
		float d1 = invMassSum;

		glm::vec3 d2 = glm::cross( glm::cross( r1, relativeNorm ), r1 );
		glm::vec3 d3 = glm::cross( glm::cross( r2, relativeNorm ), r2 );
		float denominator = d1 + glm::dot( relativeNorm, d2 + d3 );

		float j = ( denominator == 0.0f ) ? 0.0f : numerator / denominator;
		if( collisionResult.contacts.size() > 0 && j != 0.0f ) {
			j /= ( float )collisionResult.contacts.size();
		}

		glm::vec3 impulse = relativeNorm * j;

		if( !bodyA->bIsStatic )
			bodyA->mVelocity = bodyA->mVelocity - impulse * invMass1;

		if( !bodyB->bIsStatic )
			bodyB->mVelocity = bodyB->mVelocity + impulse * invMass2;

		//eShapeType typeA = bodyA->GetShape()->GetShapeType();
		//eShapeType typeB = bodyB->GetShape()->GetShapeType();
		//
		//if( typeA != nPhysics::SHAPE_TYPE_PLANE )
		//	bodyA->mVelocity = bodyA->mVelocity - impulse * invMass1;
		//
		//if( typeB != nPhysics::SHAPE_TYPE_PLANE )
		//	bodyB->mVelocity = bodyB->mVelocity + impulse * invMass2;

		return;
	}

}