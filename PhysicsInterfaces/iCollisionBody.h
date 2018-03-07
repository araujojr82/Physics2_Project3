#pragma once
namespace nPhysics
{
	class iCollisionBody
	{
	public:
		enum objectTypes
		{
			RIGID_BODY = 0,
			SOFT_BODY = 1
		};

		virtual ~iCollisionBody() {}
	};
}