#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp> // <- This includes vec3 and mat4

#include "iShape.h"
#include "iCollisionBody.h"

namespace nPhysics
{
	class iSoftBody : iCollisionBody
	{
	public:
		virtual ~iSoftBody() {}

		virtual void GetNodePosition(size_t index, glm::vec3 nodePositionOut) = 0;
		virtual size_t NumNodes() = 0;

	protected:
		// not to be used!
		//iSoftBody() : iCollisionBody( SOFT_BODY ) {}
		
		//iSoftBody(const iSoftBody& other) : iCollisionBody(other) {}
		iSoftBody& operator=(const iSoftBody& other) { return *this; }
	};
}