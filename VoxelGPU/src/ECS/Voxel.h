#pragma once

#include "Components/TransformComponent.h"
#include "Components/VoxelModel.h"

struct Voxel
{
	Voxel() {}
	Voxel(const TransformComponent& T) : Transform(T) {};
	Voxel(const TransformComponent& T, const glm::vec3& size, const glm::vec3& color) : Transform(T), VoxelModel(size, color) {};

	VoxelModel VoxelModel;
	TransformComponent Transform;
};