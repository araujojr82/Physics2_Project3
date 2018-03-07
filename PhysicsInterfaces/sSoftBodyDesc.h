#pragma once

#include "sBodyDesc.h"
#include <glm\vec3.hpp>
#include <vector>

namespace nPhysics
{
	struct sTriangle
	{
		int nodeID_0;
		int nodeID_1;
		int nodeID_2;
	};

	struct sSoftBodyDesc : public sBodyDesc
	{
		//size_t can be a structure that represents triangles
		std::vector<glm::vec3> Vertices;
		std::vector<sTriangle*> TriangulatedIndices;
		// The points that are static (dont move)
		std::vector<int> StaticIndices; 
	};
}