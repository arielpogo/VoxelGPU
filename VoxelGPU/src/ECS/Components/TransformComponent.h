#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

struct TransformComponent
{
	glm::vec3 Translation = glm::vec3(0.0f);
	glm::vec3 Rotation = glm::vec3(0.0f);
	glm::vec3 Scale = glm::vec3(1.0f);

	TransformComponent(){}
	TransformComponent(const TransformComponent&) = default;
	TransformComponent(const glm::vec3& t) : Translation(t) {};
	~TransformComponent() = default;

	inline glm::mat4 GetTransformMatrix() const
	{
		glm::mat4 toReturn(1.0f);
		toReturn[3].x = Translation.x;
		toReturn[3].y = Translation.y;
		toReturn[3].z = Translation.z;
		return toReturn;
	}
};