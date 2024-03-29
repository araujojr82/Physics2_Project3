#pragma once

#include <iCollisionBody.h>
#include <iSoftBody.h>
#include <sSoftBodyDesc.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp> // <- This includes vec3 and mat4

#include "cRigidBody.h"
#include "shapes.h"

namespace nPhysics
{

	class cSoftBody : public iSoftBody
	{
	private:

		class cNode;

		class cSpring
		{
		public:
			cSpring(cNode* nodeA, cNode* nodeB);

			cNode* GetOther(cNode* me);

			void ApplyForce();

			float RestingSeparation;	// Set at constr
			float CurrentSeparation;	// Set at constr
			glm::vec3 NormalizedSeparationDirection;

			float SpringConstantK;

			cNode* NodeA;
			cNode* NodeB;

		};

		class cNode : public cRigidBody
		{
		public:
			// Call the superclass constructor in the subclass constructor
			cNode( const sRigidBodyDesc& desc, iShape* shape ) : cRigidBody( desc, shape )
			{
				//this->IsStatic = false;
				//this->Mass = 0.0f;
				//this->Radius = 0.0f;
				//this->Position = glm::vec3( 0.0f );
				//this->Velocity = glm::vec3( 0.0f );
				//this->Acceleration = glm::vec3( 0.0f );
			}

			bool HasNeighbour(cNode * node);	// Check to see if already has a connection

			std::vector<cSpring*> Springs;

			// std::vector<cNode*> Neighbors; The Springs knows the neighbors

			//bool IsStatic;
			//float Mass;
			//float Radius;
			//glm::vec3 Position;
			//glm::vec3 Velocity;
			//glm::vec3 Acceleration;

		};

	public:

		eObjectType myType;

		cSoftBody(const sSoftBodyDesc& desc);
		void GetAABB(glm::vec3& minBoundsOut, glm::vec3& maxBoundsOut);
		// Get miminum height - radius
		// Get maximum height + radius (DONT FORGET RADIUS)

		virtual void GetNodePosition( size_t index, glm::vec3 &nodePositionOut );
		virtual size_t NumNodes();

		virtual eObjectType getType();

		void ApplySpringForces();
		void ApplyForce( glm::vec3 force );

		std::vector<cRigidBody*> getNodeListAsRigidBodies();

	protected:
		std::vector<cNode*> mNodes;
		//std::vector<cSpring*> mSprings;
		std::vector<cSpring*> mStructural;
		std::vector<cSpring*> mShear;
		std::vector<cSpring*> mBend;

	};

}

