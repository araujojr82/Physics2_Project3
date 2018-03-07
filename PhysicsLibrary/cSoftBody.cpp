#include "cSoftBody.h"

namespace nPhysics
{

	cSoftBody::cSoftBody( const sSoftBodyDesc& desc )
	{

		//desc.StaticIndices
		//desc.TriangulatedIndices
		//desc.Vertices

		//this->mNodes
		//this->mSprings
	}


	cSoftBody::~cSoftBody()
	{
	}

	// Check every node to get the minimum and maximum positions of x, y and z
	void cSoftBody::GetAABB(glm::vec3& minBoundsOut, glm::vec3& maxBoundsOut)
	{		

		for (int i = 0; i != this->mNodes.size(); i++)
		{
			if (i = 0)
			{
				minBoundsOut = this->mNodes[i]->Position;
				maxBoundsOut = this->mNodes[i]->Position;
			}
			else
			{
				if (this->mNodes[i]->Position.x < minBoundsOut.x)
					minBoundsOut.x = this->mNodes[i]->Position.x;
				if (this->mNodes[i]->Position.y < minBoundsOut.y)
					minBoundsOut.y = this->mNodes[i]->Position.y;
				if (this->mNodes[i]->Position.z < minBoundsOut.z)
					minBoundsOut.z = this->mNodes[i]->Position.z;

				if (this->mNodes[i]->Position.x > maxBoundsOut.x)
					maxBoundsOut.x = this->mNodes[i]->Position.x;
				if (this->mNodes[i]->Position.y > maxBoundsOut.y)
					maxBoundsOut.y = this->mNodes[i]->Position.y;
				if (this->mNodes[i]->Position.z > maxBoundsOut.z)
					maxBoundsOut.z = this->mNodes[i]->Position.z;
			}
		}
		return;
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