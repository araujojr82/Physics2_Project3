#include "cSoftBody.h"

namespace nPhysics
{

	cSoftBody::cSoftBody( const sSoftBodyDesc& desc )
	{
		this->myType = SOFT_BODY;

		// Calculate the radius based on first triangle
		glm::vec3 vertA = desc.Vertices[desc.TriangulatedIndices[0]->nodeID_0];
		glm::vec3 vertB = desc.Vertices[desc.TriangulatedIndices[0]->nodeID_1];

		float radius = glm::distance( vertA, vertB ) / 2;

		// Create one node for each vertice
		for( int i = 0; i != desc.Vertices.size(); i++ )
		{			
			cNode* newNode = new cNode();
			newNode->Position = desc.Vertices[i];
			newNode->Mass = 0.1f;
			newNode->Radius = radius;
			this->mNodes.push_back( newNode );
		}

		// Create the springs based on the triangles
		for ( int i = 0; i != desc.TriangulatedIndices.size(); i++ )
		{
			cNode* node0 = this->mNodes[desc.TriangulatedIndices[i]->nodeID_0];
			cNode* node1 = this->mNodes[desc.TriangulatedIndices[i]->nodeID_1];
			cNode* node2 = this->mNodes[desc.TriangulatedIndices[i]->nodeID_2];

			// Check if node already has that spring if not create one and attach to both nodes
			if( !node0->HasNeighbour( node1 ) )
			{
				cSpring* spring01 = new cSpring( node0, node1 );
				node0->Springs.push_back( spring01 );
				node1->Springs.push_back( spring01 );
				this->mSprings.push_back( spring01 );
			}
				
			if( !node0->HasNeighbour( node2 ) )
			{
				cSpring* spring02 = new cSpring( node0, node2 );
				node0->Springs.push_back( spring02 );
				node1->Springs.push_back( spring02 );
				this->mSprings.push_back( spring02 );
			}
				
			if( !node1->HasNeighbour( node2 ) )
			{
				cSpring* spring12 = new cSpring( node1, node2 );
				node0->Springs.push_back( spring12 );
				node1->Springs.push_back( spring12 );
				this->mSprings.push_back( spring12 );
			}
		}

		// Set the static nodes
		for( int i = 0; i != desc.StaticIndices.size(); i++ )
		{
			this->mNodes[desc.StaticIndices[i]]->IsStatic = true;
		}
	}

	// Check every node to get the minimum and maximum positions of x, y and z
	void cSoftBody::GetAABB(glm::vec3& minBoundsOut, glm::vec3& maxBoundsOut)
	{		
		glm::vec3 minPos = glm::vec3( 0.0f );
		glm::vec3 maxPos = glm::vec3( 0.0f );

		for (int i = 0; i != this->mNodes.size(); i++)
		{
			if (i = 0)
			{
				minBoundsOut = this->mNodes[i]->Position - this->mNodes[i]->Radius;
				maxBoundsOut = this->mNodes[i]->Position + this->mNodes[i]->Radius;
			}
			else
			{
				minPos = this->mNodes[i]->Position - this->mNodes[i]->Radius;
				maxPos = this->mNodes[i]->Position - this->mNodes[i]->Radius;

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

	void cSoftBody::GetNodePosition( size_t index, glm::vec3 nodePositionOut )
	{
		if( index < this->mNodes.size() )
			nodePositionOut = this->mNodes[index]->Position;

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

	cSoftBody::cSpring::cSpring( cNode * nodeA, cNode * nodeB )
	{
		this->SpringConstantK = 0.3f;
		this->NodeA = nodeA;
		this->NodeB = nodeB;
		this->CurrentSeparation = glm::distance( this->NodeA->Position, this->NodeB->Position );
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

	cSoftBody::cNode::cNode()
	{
		this->IsStatic = false;
		this->Mass = 0.0f;
		this->Radius = 0.0f;
		this->Position = glm::vec3( 0.0f );
		this->Velocity = glm::vec3( 0.0f );
		this->Acceleration = glm::vec3( 0.0f );

		return;
	}

	bool cSoftBody::cNode::HasNeighbour(cNode * node)
	{
		return false;
	}

}