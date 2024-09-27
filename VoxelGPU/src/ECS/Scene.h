#pragma once

#include "vendor/entt.hpp"
#include "Components/TransformComponent.h"

class Scene
{
entt::registry registry;

public:
	Scene()
	{
		entt::entity e = registry.create(); //a uint32_t

		registry.emplace<TransformComponent>(e);
	}

	~Scene()
	{

	}
};