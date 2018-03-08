#pragma once

#include "iCollisionBody.h"

namespace nPhysics
{
	class cCollisionBody : public iCollisionBody
	{
	public:
		//enum eObjectTypes
		//{
		//	RIGID_BODY = 0,
		//	SOFT_BODY = 1
		//};

		virtual ~cCollisionBody() {}

		virtual eObjectType getType() = 0;
	};
}