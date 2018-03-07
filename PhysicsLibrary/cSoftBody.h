#pragma once

#include <iSoftBody.h>
#include <sSoftBodyDesc.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp> // <- This includes vec3 and mat4

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

			float RestingSeparation;	// Set at constr
			float CurrentSeparation;	// Set at constr
			glm::vec3 NormalizedSeparationDirection;

			float SpringConstantK;

			cNode* NodeA;
			cNode* NodeB;

		};

		class cNode
		{
		public:
			cNode();

			bool HasNeighbour(cNode * node);	// Check to see if already has a connection

			std::vector<cSpring*> Springs;

			// std::vector<cNode*> Neighbors; The Springs knows the neighbors

			bool IsStatic;
			float Mass;
			float Radius;
			glm::vec3 Position;
			glm::vec3 Velocity;
			glm::vec3 Acceleration;

		};

	public:

		eObjectType myType;

		cSoftBody(const sSoftBodyDesc& desc);
		void GetAABB(glm::vec3& minBoundsOut, glm::vec3& maxBoundsOut);
		// Get miminum height - radius
		// Get maximum height + radius (DONT FORGET RADIUS)

		virtual void GetNodePosition( size_t index, glm::vec3 nodePositionOut );
		virtual size_t NumNodes();

		virtual eObjectType getType();

	protected:
		std::vector<cNode*> mNodes;
		std::vector<cSpring*> mSprings;

	};

}

