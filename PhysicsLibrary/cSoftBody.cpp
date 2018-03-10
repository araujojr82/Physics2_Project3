#include "cSoftBody.h"

namespace nPhysics
{

	cSoftBody::cSoftBody( const sSoftBodyDesc& desc )
	{
		this->myType = SOFT_BODY;

		//// Calculate the radius based on first triangle
		//glm::vec3 vertA = desc.Vertices[desc.TriangulatedIndices[0]->nodeID_0];
		//glm::vec3 vertB = desc.Vertices[desc.TriangulatedIndices[0]->nodeID_1];

		//float radius = glm::distance( vertA, vertB ) / 2;

		// I'll use the same sphereShape for every node
		//iShape* sphereShape = new cSphereShape( 0.0f );
		iShape* particleShape = new cParticleShape();

		// Create one node for each vertice
		for( int i = 0; i != desc.Vertices.size(); i++ )
		{			
			sRigidBodyDesc theDesc;

			theDesc.Position = desc.Vertices[i];
			theDesc.Mass = 1.0f; //0.001f;

			if( theDesc.Mass == 0.0f )
				theDesc.invMass = theDesc.Mass;
			else
				theDesc.invMass = 1 / theDesc.Mass;

			theDesc.PrevPosition = theDesc.Position;

			cNode* newNode = new cNode( theDesc, particleShape );
			newNode->setParticle( true );
			this->mNodes.push_back( newNode );
		}

		// Create the springs based on the constrains
		for ( int i = 0; i != desc.ConstrainIndices.size(); i++ )
		{
			cNode* node0 = this->mNodes[desc.ConstrainIndices[i]->nodeID_0];
			cNode* node1 = this->mNodes[desc.ConstrainIndices[i]->nodeID_1];

			// Check if node already has that spring if not create one and attach to both nodes
			if( !node0->HasNeighbour( node1 ) )
			{
				cSpring* spring01 = new cSpring( node0, node1 );
				node0->Springs.push_back( spring01 );
				node1->Springs.push_back( spring01 );
				//this->mSprings.push_back( spring01 );

				switch( desc.ConstrainIndices[i]->type )
				{
					case STRUCTURAL :
						this->mStructural.push_back( spring01 );
						break;
					case SHEAR :
						this->mShear.push_back( spring01 );
						break;
					case BEND :
						//this->mBend.push_back( spring01 );
						break;					
				}
			}
		}

		// Set the static nodes
		for( int i = 0; i != desc.StaticIndices.size(); i++ )
		{
			this->mNodes[desc.StaticIndices[i]]->setStatic( true );
		}
	}

	// Check every node to get the minimum and maximum positions of x, y and z
	void cSoftBody::GetAABB(glm::vec3& minBoundsOut, glm::vec3& maxBoundsOut)
	{		
		glm::vec3 minPos = glm::vec3( 0.0f );
		glm::vec3 maxPos = glm::vec3( 0.0f );

		for (int i = 0; i != this->mNodes.size(); i++)
		{
			iShape* theShape = this->mNodes[i]->GetShape();

			glm::vec3 nodePosition = glm::vec3( 0.0f );
			this->mNodes[i]->GetPosition( nodePosition );

			float radius = 0.0f;
			theShape->GetSphereRadius( radius );

			if (i = 0)
			{
				minBoundsOut = nodePosition - radius;
				maxBoundsOut = nodePosition + radius;
			}
			else
			{			
				minPos = nodePosition - radius;
				maxPos = nodePosition + radius;

				if( minPos.x < minBoundsOut.x )
					minBoundsOut.x = minPos.x;
				if (minPos.y < minBoundsOut.y)
					minBoundsOut.y = minPos.y;
				if (minPos.z < minBoundsOut.z)
					minBoundsOut.z = minPos.z;

				if (maxPos.x > maxBoundsOut.x)
					maxBoundsOut.x = maxPos.x;
				if (maxPos.y > maxBoundsOut.y)
					maxBoundsOut.y = maxPos.y;
				if (maxPos.z > maxBoundsOut.z)
					maxBoundsOut.z = maxPos.z;
			}
		}
		return;
	}

	void cSoftBody::GetNodePosition( size_t index, glm::vec3 &nodePositionOut )
	{
		if( index < this->mNodes.size() )
		{
			this->mNodes[index]->GetPosition( nodePositionOut );
		}			

		return;
	}

	size_t cSoftBody::NumNodes()
	{
		return this->mNodes.size();
	}

	eObjectType cSoftBody::getType()
	{
		return this->myType;
	}

	void cSoftBody::ApplySpringForces()
	{
		for( int times = 0; times < 15; times++ ) // iterate over all constraints several times
		{
			for( int i = 0; i != mStructural.size(); i++ )
			{
				mStructural[i]->ApplyForce();
			}

			for( int i = 0; i != mShear.size(); i++ )
			{
				mShear[i]->ApplyForce();
			}

			for( int i = 0; i != mBend.size(); i++ )
			{
				mBend[i]->ApplyForce();
			}
		}
	}

	void cSoftBody::ApplyForce( glm::vec3 force )
	{
		for( int i = 0; i != mNodes.size(); i++ )
		{
			mNodes[i]->ApplyForce( force );
		}
	}

	std::vector<cRigidBody*> cSoftBody::getNodeListAsRigidBodies()
	{
		std::vector<cRigidBody*> theRigidBodies;

		for( int i = 0; i != this->mNodes.size(); i++ )
		{
			//iRigidBody* theBody = dynamic_cast< iRigidBody* > ( this->mNodes[i] );
			theRigidBodies.push_back( this->mNodes[i] );
		}

		return theRigidBodies;
	}

	cSoftBody::cSpring::cSpring( cNode * nodeA, cNode * nodeB )
	{
		this->SpringConstantK = 1.0f;
		this->NodeA = nodeA;
		this->NodeB = nodeB;

		glm::vec3 posA;
		glm::vec3 posB;

		this->NodeA->GetPosition( posA );
		this->NodeB->GetPosition( posB );

		this->CurrentSeparation = glm::distance( posA, posB );
		this->RestingSeparation = this->CurrentSeparation;
			
		return;
	}

	cSoftBody::cNode* cSoftBody::cSpring::GetOther( cNode* me )
	{
		if ( this->NodeA == me )
			return this->NodeB;
		else
			return this->NodeA;

		return nullptr;
	}

	void cSoftBody::cSpring::ApplyForce()
	{
		glm::vec3 posA, posB, velA, velB;

		this->NodeA->GetPosition( posA );
		this->NodeB->GetPosition( posB );

		this->NodeA->GetVelocity( velA );
		this->NodeB->GetVelocity( velB );

		glm::vec3 relPos = posB - posA;
		glm::vec3 relVel = velB - velA;

		float current_distance = glm::length(relPos);
		
		// The offset vector that could moves p1 into a distance of rest_distance to p2
		glm::vec3 correctionVector = relPos * ( 1 - this->RestingSeparation / current_distance ); 

		// Lets make it half that length, so that we can move BOTH p1 and p2.
		glm::vec3 correctionVectorHalf = correctionVector * 0.5f;

		if( correctionVectorHalf != glm::vec3( 0.0f ) )
		{
			int breakpoint = 1;
		}

		posA = posA + correctionVectorHalf;
		posB = posB - correctionVectorHalf;
		
		if( !this->NodeA->isStatic() )		
			this->NodeA->SetPosition( posA );
		if( !this->NodeB->isStatic() )		
			this->NodeB->SetPosition( posB );

		//// Prevent underflow
		//for( int i = 0; i < 3; ++i )
		//{
		//	relPos[i] = ( fabsf( relPos[i] ) < 0.0000001f ) ? 0.0f : relPos[i];
		//	relVel[i] = ( fabsf( relVel[i] ) < 0.0000001f ) ? 0.0f : relVel[i];
		//}

		//float x = glm::length( relPos ) - this->RestingSeparation;
		//float v = glm::length( relVel );

		//float b = 0.0f;

		//if( x != 0.0f || v != 0.0f )
		//{
		//	int breakbpoint = 0;
		//}

		//float F = ( -this->SpringConstantK * x ) + ( -b * v );

		//float invMassA, invMassB;
		//this->NodeA->getInvMass( invMassA );
		//this->NodeB->getInvMass( invMassB );

		//glm::vec3 impulse = glm::normalize( relPos ) * F;

		//if( !this->NodeA->isStatic() )
		//this->NodeA->ApplyImpulse( impulse * invMassA );

		//if( !this->NodeB->isStatic() )
		//this->NodeB->ApplyImpulse( impulse*  -1.0f * invMassB );

	}

	//cSoftBody::cNode::cNode( const sRigidBodyDesc& desc, iShape* shape ) : cRigidBody( desc, shape )
	//{
	//	this->IsStatic = false;
	//	this->Mass = 0.0f;
	//	this->Radius = 0.0f;
	//	this->Position = glm::vec3( 0.0f );
	//	this->Velocity = glm::vec3( 0.0f );
	//	this->Acceleration = glm::vec3( 0.0f );

	//	return;
	//}

	bool cSoftBody::cNode::HasNeighbour(cNode * node)
	{
		return false;
	}

}