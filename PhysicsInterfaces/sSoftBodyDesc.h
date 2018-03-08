#pragma once

#include "sBodyDesc.h"
#include <glm\vec3.hpp>
#include <vector>

namespace nPhysics
{
	enum eConstrainType
	{
		STRUCTURAL = 0,
		SHEAR = 1,
		BEND = 2
	};

	struct sTriangle
	{
		int nodeID_0;
		int nodeID_1;
		int nodeID_2;
	};

	struct sConstrain
	{
		eConstrainType type;
		int nodeID_0;
		int nodeID_1;
	};

	struct sSoftBodyDesc : public sBodyDesc
	{
		
		std::vector<glm::vec3> Vertices;
		std::vector<sTriangle*> TriangulatedIndices;

		std::vector<sConstrain*> ConstrainIndices;
		
		std::vector<int> StaticIndices; // The points that are static (dont move)

		float radius;
	};
}