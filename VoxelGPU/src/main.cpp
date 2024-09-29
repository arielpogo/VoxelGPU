#include "Renderer.h"
#include "ECS/Scene.h"

int main(){
	try 
	{
		Renderer renderer;
		GLFWwindow* window;

		window = renderer.getWindowPointer();

		Scene scene(renderer.getDeviceHandler(), renderer.getCommandBuffersHandler());

		scene.AddVoxel(glm::vec3(0.0f, 1.0f, 0.0f));
		scene.AddVoxel(glm::vec3(5.0f, 0.0f, 0.0f));

		scene.FinishScene();

		renderer.scene = &scene;

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