#pragma once

#include "Components/TransformComponent.h"
#include "Components/VoxelModel.h"

struct Voxel
{
	Voxel() {};

	VoxelModel VoxelModel;
	TransformComponent Transform;
};