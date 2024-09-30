#pragma once


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "vulkanHandlers/UniformBuffers.h"
#include "vulkanHandlers/SwapchainHandler.h"

class Camera {
	const glm::mat4 correction = glm::mat4(
        glm::vec4(1.0f,  0.0f, 0.0f, 0.0f),
        glm::vec4(0.0f, -1.0f, 0.0f, 0.0f),
        glm::vec4(0.0f,  0.0f, 0.5f, 0.0f),
        glm::vec4(0.0f,  0.0f, 0.5f, 1.0f));

	const float cameraSpeed = 0.20f;
	const float shiftSpeedModifier = 0.30f; //added, not multiplied
	const float ctrlSpeedModifier = 10.0f; //divided by this
	const float sensitivity = 0.05f;
	float lastx;
	float lasty;
	bool firstMouseInput = true;

	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f,-2.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	float yaw =   90.0f;
	float pitch = 0.0f;

	UniformBuffers* uniformBuffers;
	UniformBufferObject ubo;
	SwapchainHandler* swapchainHandler;

public:
	Camera(DeviceHandler* _dh, SwapchainHandler* _sh) : swapchainHandler(_sh) {
		uniformBuffers = new UniformBuffers(_dh, _sh);
		ubo.model = glm::mat4(1.0f);
		ubo.view = glm::mat4(1.0f);
		ubo.projection = correction * glm::perspective(glm::radians(45.0f), swapchainHandler->getSwapchainExtent().width / (float)swapchainHandler->getSwapchainExtent().height, 0.1f, 1000.0f);
		//ubo.projection[1][1] *= -1; //glm was originally for opengl which has the y clip coordinates inverted from Vulkan
	}

	~Camera()
	{
		delete uniformBuffers;
	}

	inline UniformBuffers* getUniformBuffers() { return uniformBuffers; }
	inline glm::vec3& getPos() { return cameraPos; }
	inline glm::vec3& getCameraDirection() { return cameraDirection; }

	void Update(uint32_t currentFrame) {
		ubo.model = glm::mat4(1.0f);
		ubo.projection = glm::perspective(glm::radians(45.0f), swapchainHandler->getSwapchainExtent().width / (float)swapchainHandler->getSwapchainExtent().height, 0.1f, 10.0f);
		ubo.projection[1][1] *= -1; //glm was originally for opengl which has the y clip coordinates inverted from Vulkan
		ubo.view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

		uniformBuffers->updateUniformBuffer(ubo, currentFrame);
	}

	void MouseCallback(float xpos, float ypos)
	{
		if (firstMouseInput) {
			lastx = xpos;
			lasty = ypos;
			firstMouseInput = false;
		}

		float xoffset = xpos - lastx;
		float yoffset = lasty - ypos; //reversed since y ranges from bottom to top
		lastx = xpos;
		lasty = ypos;

		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		if (pitch > 89.0f) pitch = 89.0f;
		else if (pitch < -89.0f) pitch = -89.0f;

		cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraDirection.y = sin(glm::radians(pitch));
		cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		//std::cout << cameraDirection.x << ' ' << cameraDirection.y << ' ' << cameraDirection.z << '\n';
		//std::cout << '\t' << yaw << ' ' << pitch << '\n';
		cameraFront = glm::normalize(cameraDirection);
	}

	void InputCallback(GLFWwindow* window)
	{
		float finalSpeed = cameraSpeed;

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) 
			finalSpeed += shiftSpeedModifier;
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) 
			finalSpeed /= ctrlSpeedModifier;

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			cameraPos += finalSpeed * cameraFront;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			cameraPos -= finalSpeed * cameraFront;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * finalSpeed;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * finalSpeed;
	}
};