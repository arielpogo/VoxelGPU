#include "Renderer.h"
#include "ECS/Scene.h"
#include <random>

int main(){
	try 
	{
		Renderer renderer;
		GLFWwindow* window;

		window = renderer.getWindowPointer();

		Scene scene(renderer.getDeviceHandler(), renderer.getCommandBuffersHandler());

		std::linear_congruential_engine<std::uint_fast32_t, 16807, 0, 2147483647> lce;
		glm::vec3 white(1.0f);
		glm::vec3 blue(0.0f, 0.0f, 1.0f);

		unsigned int num = 0;
		for (float x = 0.0f; x < 10.0f; x += 0.1f)
		{
			for (float z = 0.0f; z < 10.0f; z += 0.1f)
			{
				glm::vec3 position(x, 0.0f, z);
				if(num % 2 == 0) scene.AddVoxel(TransformComponent(position), white);
				else scene.AddVoxel(TransformComponent(position), blue);
				++num;
			}
			++num;
		}

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