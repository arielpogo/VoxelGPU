
#include "Renderer.h"

int main(){
	Renderer renderer;
	GLFWwindow* window;

	try 
	{
		renderer.init();
		window = renderer.getWindowPointer();

		while (!glfwWindowShouldClose(window))
		{
			renderer.doLoop();
		}

		renderer.terminate();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}