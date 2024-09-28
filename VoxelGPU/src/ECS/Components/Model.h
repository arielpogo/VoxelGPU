#pragma once

#include <vector>
#include <stdint.h>

#include "src/Vertex.h"

class Model 
{
protected:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

public:
	inline Vertex* getVertexData() { return vertices.data(); }
	inline std::size_t getVertexDataSize() { return vertices.size(); }
	inline uint32_t* getIndicesData() { return indices.data(); }
	inline std::size_t getIndicesDataSize() { return indices.size(); }
};