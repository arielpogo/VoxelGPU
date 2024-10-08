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
	TransformComponent(const glm::vec3& t, const glm::vec3& r = glm::vec3(0.0f), const glm::vec3& s = glm::vec3(1.0f)) : Translation(t), Rotation(r), Scale(s) {};

	inline glm::mat4 GetTransformMatrix() const
	{	
		return glm::translate(glm::mat4(1.0f), Translation);
	}
};