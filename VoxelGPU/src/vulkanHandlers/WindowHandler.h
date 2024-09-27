#pragma once


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
//vulkan.h is loaded above

#include <glm/glm.hpp>

class WindowHandler{
    GLFWwindow* window;

public:
    inline GLFWwindow* getWindowPointer() const { return window; }

    WindowHandler(){
		glfwInit(); //init glfw library
		
		//do not create an OpenGL context
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(WIDTH, HEIGHT, "VoxelGPU", nullptr, nullptr);
	}

    ~WindowHandler(){
        glfwDestroyWindow(window);
        glfwTerminate();
    }
};