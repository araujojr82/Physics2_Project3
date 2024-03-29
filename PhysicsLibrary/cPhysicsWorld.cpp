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
			cRigidBody* theRigidBody = dynamic_cast< cRigidBody* > ( collisionBody );
			std::vector<cRigidBody*>::iterator itRigidBody;
			itRigidBody = std::find( mRigidBody.begin(), mRigidBody.end(), theRigidBody );
			if( itRigidBody == mRigidBody.end() )
			{
				mRigidBody.push_back( theRigidBody );
			}
		}

		else
		{	// A soft body is made of rigid body nodes, so we need to add them
			cSoftBody* theSoftBody = dynamic_cast< cSoftBody* > ( collisionBody );
			
			std::vector<cRigidBody*> theSBNodes = theSoftBody->getNodeListAsRigidBodies();

			for( int i = 0; i != theSBNodes.size(); i++ )
			{
				mRigidBody.push_back( theSBNodes[i] );
			}

			mSoftyBody.push_back( theSoftBody );
		}
		
	}

	void cPhysicsWorld::RemoveBody( iCollisionBody* collisionBody )
	{
		eObjectType bodyType = collisionBody->getType();

		if( bodyType == RIGID_BODY )
		{
			cRigidBody* theRigidBody = dynamic_cast< cRigidBody* > ( collisionBody );
			
			std::vector<cRigidBody*>::iterator itRigidBody;
			
			itRigidBody = std::remove( mRigidBody.begin(), mRigidBody.end(), theRigidBody );
			if( itRigidBody != mRigidBody.end() )
			{
				mRigidBody.resize( mRigidBody.size() - 1 );
			}
		}

		else
		{	// A soft body is made of rigid body nodes, so we need to remove them
			cSoftBody* theSoftBody = dynamic_cast< cSoftBody* > ( collisionBody );
			
			std::vector<cRigidBody*> theSBNodes = theSoftBody->getNodeListAsRigidBodies();

			for( int i = 0; i != theSBNodes.size(); i++ )
			{
				std::vector<cRigidBody*>::iterator itRigidBody;

				itRigidBody = std::remove( mRigidBody.begin(), mRigidBody.end(), theSBNodes[i] );
				if( itRigidBody != mRigidBody.end() )
				{
					mRigidBody.resize( mRigidBody.size() - 1 );
				}
			}
			mSoftyBody.push_back( theSoftBody );
		}
	}

	// Utility Function
	glm::vec3 ClosestPoint( glm::vec3 planeNormal, float planeConst, glm::vec3 thePoint )
	{
		float distance = glm::dot( planeNormal, thePoint ) - planeConst;
		return thePoint - planeNormal * distance;
	}

	// Utility Function
	bool CMP( float x, float y )
	{
		return ( fabsf( x - y ) <= FLT_EPSILON * fmaxf( 1.0f, fmaxf( fabsf( x ), fabsf( y ) ) ) );
	}

	void cPhysicsWorld::TimeStep( float deltaTime )
	{
		collisionResults.clear();
		vecColliders1.clear();
		vecColliders2.clear();	

		int numBodies = mRigidBody.size();

		for( int i = 0; i != numBodies - 1; i++ )
		{
			for( int j = i+1; j != numBodies; j++ )
			{
				sCollisionManifold result;
				result = FindCollision( mRigidBody[i], mRigidBody[j] );

				if( result.colliding )
				{
					vecColliders1.push_back( mRigidBody[i] );
					vecColliders2.push_back( mRigidBody[j] );
					collisionResults.push_back( result );
				}
			}
		}

		// Apply forces
		for( int i = 0; i != numBodies; i++ )
		{
			eObjectType bodyTypeA = mRigidBody[i]->getType();

			cRigidBody* theBody = dynamic_cast< cRigidBody* > ( mRigidBody[i] );

			if( theBody->bIsStatic ) continue;

			eShapeType type = theBody->GetShape()->GetShapeType();
			if( type != nPhysics::SHAPE_TYPE_PLANE )
				theBody->ApplyForce( GRAVITY );			
		}

		//Apply spring forces for softbodies
		for( int i = 0; i != mSoftyBody.size(); ++i )
		{
			mSoftyBody[i]->ApplySpringForces();
		}

		// Integrate velocity and impulse of objects
		for( int i = 0; i != numBodies; i++ )
		{
			eObjectType bodyTypeA = mRigidBody[i]->getType();
			cRigidBody* theBody = NULL;

			if( bodyTypeA == RIGID_BODY )
			{
				theBody = dynamic_cast< cRigidBody* > ( mRigidBody[i] );

				if( theBody->bIsStatic ) continue;

				//cRigidBody* theBody = mRigidBody[i];
				glm::vec3 force = theBody->mAcceleration;
				glm::vec3 acceleration = force * theBody->mInvMass;

				eShapeType type = theBody->GetShape()->GetShapeType();
				switch( type )
				{
					case nPhysics::SHAPE_TYPE_PLANE:
						// Nothing to do
						break;
					case nPhysics::SHAPE_TYPE_SPHERE:
						this->myIntegrator.RK4( theBody->mPosition, theBody->mVelocity, acceleration, deltaTime );
						break;
					case nPhysics::SHAPE_TYPE_PARTICLE:
						this->myIntegrator.Verlet( theBody->mPosition, theBody->mPrevPosition, acceleration, deltaTime );
						break;
					default:
						break;
				}					
			}
		}

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

		// Correct position to avoid sinking!
		for( int i = 0; i != collisionResults.size(); i++ )
		{
			eObjectType bodyTypeA = vecColliders1[i]->getType();
			eObjectType bodyTypeB = vecColliders2[i]->getType();

			cRigidBody* m1 = NULL;
			cRigidBody* m2 = NULL;

			m1 = dynamic_cast< cRigidBody* > ( vecColliders1[i] );

			m2 = dynamic_cast< cRigidBody* > ( vecColliders2[i] );

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

		vecColliders1.clear();
		vecColliders2.clear();
		collisionResults.clear();

		// Check for cloth collisions
		for( int i = 0; i != numBodies - 1; i++ )
		{
			for( int j = i + 1; j != numBodies; j++ )
			{
				ParticleCollision( mRigidBody[i], mRigidBody[j] );
			}
		}

		// Clear acceleration
		for( int i = 0; i != numBodies; i++ ) 
		{
			eObjectType bodyTypeA = mRigidBody[i]->getType();
			cRigidBody* theBody = NULL;

			theBody = dynamic_cast< cRigidBody* > ( mRigidBody[i] );
			theBody->mAcceleration = glm::vec3( 0.0f );
		}
	}

	sCollisionManifold cPhysicsWorld::FindCollision( cRigidBody* bodyA, cRigidBody* bodyB )
	{
		sCollisionManifold result;
		
		eShapeType typeA = bodyA->GetShape()->GetShapeType();
		eShapeType typeB = bodyB->GetShape()->GetShapeType();

		if( typeA == nPhysics::SHAPE_TYPE_SPHERE )
		{
			cSphereShape * sphereA = dynamic_cast< cSphereShape* > ( bodyA->GetShape() );

			if( typeB == nPhysics::SHAPE_TYPE_SPHERE )
			{
				cSphereShape * sphereB = dynamic_cast< cSphereShape* > ( bodyB->GetShape() );
				
				result = FindColSphereSphere( bodyA, sphereA, bodyB, sphereB );
			}
			else if( typeB == nPhysics::SHAPE_TYPE_PLANE )
			{
				cPlaneShape* planeB = dynamic_cast< cPlaneShape* >( bodyB->GetShape() );

				result = FindColSpherePlane( bodyA, sphereA, bodyB, planeB );
				result.normal = result.normal * -1.0f;
			}
		}
		else if( typeA == nPhysics::SHAPE_TYPE_PLANE )
		{
			cPlaneShape* planeA = dynamic_cast< cPlaneShape* >( bodyA->GetShape() );
			if( typeB == nPhysics::SHAPE_TYPE_PLANE )
			{
				// Do Nothing
			}
			else if( typeB == nPhysics::SHAPE_TYPE_SPHERE )
			{
				cSphereShape * sphereB = dynamic_cast< cSphereShape* > ( bodyB->GetShape() );
				result = FindColSpherePlane( bodyB, sphereB, bodyA, planeA );
			}
		}

		return result;
	}

	void cPhysicsWorld::ParticleCollision( cRigidBody* bodyA, cRigidBody* bodyB )
	{

		eShapeType typeA = bodyA->GetShape()->GetShapeType();
		eShapeType typeB = bodyB->GetShape()->GetShapeType();
		
		if( typeA == nPhysics::SHAPE_TYPE_PARTICLE || typeB == nPhysics::SHAPE_TYPE_PARTICLE )
		{	// One of the Rigid bodies must be a particle

			if( typeA == nPhysics::SHAPE_TYPE_PARTICLE && typeB == nPhysics::SHAPE_TYPE_PARTICLE )
			{	// Don't collide particles with each other
				return;
			}

			glm::vec3 posA;
			bodyA->GetPosition( posA );

			glm::vec3 posB;
			bodyB->GetPosition( posB );

			if( typeA == nPhysics::SHAPE_TYPE_PARTICLE )		// Body A is the particle
			{
				if( typeB == nPhysics::SHAPE_TYPE_SPHERE )
				{
					cSphereShape * sphereB = dynamic_cast< cSphereShape* > ( bodyB->GetShape() );
					float radiusB;
					sphereB->GetSphereRadius( radiusB );

					glm::vec3 v = posA - posB;
					float length = glm::length( v );
					if( length < radiusB ) // if the particle is inside the ball
					{
						// project the particle to the surface of the ball
						posA += ( glm::normalize( v ) * ( radiusB - length ) );
						bodyA->SetPosition( posA );
					}

				}
			}
			else													// Body B is the particle
			{
				if( typeA == nPhysics::SHAPE_TYPE_SPHERE )
				{
					cSphereShape * sphereA = dynamic_cast< cSphereShape* > ( bodyA->GetShape() );
					float radiusA;
					sphereA->GetSphereRadius( radiusA );

					glm::vec3 v = posB - posA;
					float length = glm::length( v );
					if( length < radiusA ) // if the particle is inside the ball
					{
						// project the particle to the surface of the ball
						posB += ( glm::normalize( v ) * ( radiusA - length ) );
						bodyB->SetPosition( posB );
					}

				}				
			}
		}
		
		return;
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

	//void cPhysicsWorld::SolveCollision( iCollisionBody* bodyA, iCollisionBody* bodyB, sCollisionManifold collisionResult, int c )
	//{
	//	eObjectType bodyTypeA = bodyA->getType();
	//	eObjectType bodyTypeB = bodyB->getType();

	//	cRigidBody* rbA = NULL;
	//	cRigidBody* rbB = NULL;
	//	
	//	rbA = dynamic_cast< cRigidBody* > ( bodyA );		
	//	rbB = dynamic_cast< cRigidBody* > ( bodyB );		

	//	SolveCollision( rbA, rbB, collisionResult, c );

	//	return;
	//}

	void cPhysicsWorld::SolveCollision( cRigidBody* bodyA, cRigidBody* bodyB, sCollisionManifold collisionResult, int c )
	{

		float invMass1 = bodyA->mInvMass;
		float invMass2 = bodyB->mInvMass;
		float invMassSum = invMass1 + invMass2;

		if( invMassSum == 0.0f ) 
		{
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

		bodyA->mVelocity = bodyA->mVelocity - impulse * invMass1;
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