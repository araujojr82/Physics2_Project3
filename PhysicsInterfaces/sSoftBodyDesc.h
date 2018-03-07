#pragma once

#include <glm\vec3.hpp>
#include <vector>

namespace nPhysics
{
	struct sSoftBodyDesc
	{
		std::vector<glm::vec3> Vertices;
		std::vector<size_t> TriangulatedIndices;
		std::vector<size_t> StaticIndices; // The points that are static (dont move)
										   //size_t can be a structure that represents triangles

	};
}