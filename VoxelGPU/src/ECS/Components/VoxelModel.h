#pragma once


#include "Model.h"

class VoxelModel : public Model
{
public:
	VoxelModel(glm::vec3& size)
	{
		InitData(size.x, size.y, size.z);
	}

	VoxelModel()
	{
		InitData(1.0f, 1.0f, 1.0f);
	}

	void InitData(float l, float w, float h)
	{
		vertices =
		{
			Vertex(glm::vec3(0,0,0), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)),//0
			Vertex(glm::vec3(0,0,w), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)),//1
			Vertex(glm::vec3(0,h,0), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)),//2
			Vertex(glm::vec3(0,h,w), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)),//3
			Vertex(glm::vec3(l,0,0), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)),//4
			Vertex(glm::vec3(l,0,w), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)),//5
			Vertex(glm::vec3(l,h,0), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)),//6
			Vertex(glm::vec3(l,h,w), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f))//7
		};

		indices = {
			0,2,4, 2,6,4, //-z face
			3,0,1, 3,2,0, //-x face
			7,1,5, 7,3,1, //+z face
			6,5,4, 6,7,5, //+x face
			3,6,2, 3,7,6, //+y face
			0,5,1, 0,4,5, //-y face
		};
	}

	~VoxelModel() {}
};