#pragma once

#include "vulkanHandlers/QueueFamilyIndices.h"
#include "vulkanHandlers/SwapchainSupportDetails.h"
#include "vulkanHandlers/WindowHandler.h"
#include "vulkanHandlers/InstanceHandler.h"
#include "vulkanHandlers/SurfaceHandler.h"
#include "vulkanHandlers/DeviceHandler.h"
#include "vulkanHandlers/SwapchainHandler.h"
#include "vulkanHandlers/UniformBuffers.h"
#include "vulkanHandlers/TextureHandler.h"
#include "vulkanHandlers/BufferHelpers.h"
#include "vulkanHandlers/DescriptorSetsHandler.h"
#include "vulkanHandlers/GraphicsPipelineHandler.h"
#include "vulkanHandlers/CommandBuffersHandler.h"
#include "vulkanHandlers/DepthResourcesHandler.h"

#include "ECS/Scene.h"
#include "Vertex.h"
#include "Camera.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imconfig.h"
#include "imgui_internal.h"
#include "imstb_rectpack.h"
#include "imstb_textedit.h"
#include "imstb_truetype.h"

#include <chrono>
#include <cmath>

static void framebufferResizeCallback(GLFWwindow*, int, int);
static void mouse_callback(GLFWwindow*, double, double);
static void processInput(GLFWwindow*);

class Renderer
{
public:
	Renderer() { init(); }

	Camera* camera;
	bool framebufferResized = false;

	Scene* scene;

	GLFWwindow* getWindowPointer() { return windowHandler->getWindowPointer(); }
	DeviceHandler* getDeviceHandler() { return deviceHandler; }
	CommandBuffersHandler* getCommandBuffersHandler() { return commandBuffersHandler;  }
	
	void doLoop()
	{
		auto now = std::chrono::high_resolution_clock::now();
		auto timePassed = now - previousTimepoint;

		if (timePassed > oneSecond)
		{
			auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(timePassed);
			fps = framesRendered / (ms.count()/1000.0f);
			framesRendered = 0;
			previousTimepoint = now;
		}

		glfwPollEvents();
		drawFrame();
		++framesRendered;
	}

	void terminate() 
	{
		vkDeviceWaitIdle(deviceHandler->getLogicalDevice()); //prevent premature closure while the device is finishing up
		cleanup();
	}

	void processImGui()
	{
#ifdef DEBUG
		ImGui::Text("FPS: %.1f", fps);

		glm::vec3& pos = camera->getPos();
		ImGui::Text("Position");
		ImGui::Text("\tX: %.3f", pos.x);
		ImGui::Text("\tY: %.3f", pos.y);
		ImGui::Text("\tZ: %.3f", pos.z);

		glm::vec3& dir = camera->getCameraDirection();
		const char* directionString;
		if (std::fabs(dir.z) > std::fabs(dir.x))
		{
			if (dir.z < 0) directionString = "-Z, North";
			else directionString = "+Z, South";
		}
		else
		{
			if (dir.x < 0) directionString = "-X, West";
			else directionString = "+X, East";
		}

		ImGui::Text("Camera direction");
		ImGui::Text("\tX: %.3f", dir.x);
		ImGui::Text("\tZ: %.3f", dir.z);
		ImGui::Text("\tY: %.3f", dir.y);
		ImGui::Text("\t%s", directionString);
#endif
	}

private:
	uint32_t currentFrame = 0;

	//for fps purposes
	uint16_t framesRendered = 0;
	std::chrono::steady_clock::time_point previousTimepoint = std::chrono::high_resolution_clock::now();
	const std::chrono::seconds oneSecond = std::chrono::seconds(1);
	float fps = 0;

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
	
	WindowHandler* windowHandler;
	InstanceHandler* instanceHandler;
	SurfaceHandler* surfaceHandler;
    DeviceHandler* deviceHandler;
	SwapchainHandler* swapchainHandler;
	RenderPassHandler* renderPassHandler;
	DescriptorSetsHandler* descriptorSets;
	GraphicsPipelineHandler* graphicsPipelineHandler;
	CommandBuffersHandler* commandBuffersHandler;

	TextureHandler* texture;

	ImGuiContext* ImGuiContext;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores; //
	std::vector<VkFence> inFlightFences; //used to block host while gpu is rendering the previous frame

	void init();
	void initVulkan();
	void initImGui();
	void cleanup();

	void createSyncObjects();

	void drawFrame();
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
};

static void framebufferResizeCallback(GLFWwindow* window, int width, int height){
    Renderer* renderer = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
    renderer->framebufferResized = true; //let the renderer know the framebuffer was resized
}

static void processInput(GLFWwindow* window) {
	//call InputCallback on the window's render's camera
	reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window))->camera->InputCallback(window);
}

static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	//call MouseCallback on the window's render's camera
	reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window))->camera->MouseCallback((float) xpos, (float) ypos);
}

void Renderer::init()
{
	windowHandler = new WindowHandler();
	GLFWwindow* window = windowHandler->getWindowPointer();

	glfwSetWindowUserPointer(window, this); //so these callbacks can access the renderer (this)
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //FPS camera input
	glfwSetCursorPosCallback(window, mouse_callback);

	initVulkan();
	initImGui();
}

void Renderer::initVulkan() {
	instanceHandler = new InstanceHandler(validationLayers);
	surfaceHandler = new SurfaceHandler(instanceHandler, windowHandler);
	deviceHandler = new DeviceHandler(instanceHandler, surfaceHandler, validationLayers);
	VkDevice& logicalDevice = deviceHandler->getLogicalDevice();

	swapchainHandler = new SwapchainHandler(windowHandler, surfaceHandler, deviceHandler);
	renderPassHandler = new RenderPassHandler(logicalDevice, swapchainHandler->getSwapchainImageFormat(), swapchainHandler->findDepthFormat());
	swapchainHandler->createInitialFrameBuffers(renderPassHandler);

	commandBuffersHandler = new CommandBuffersHandler(deviceHandler);
	camera = new Camera(deviceHandler, swapchainHandler);
	texture = new TextureHandler(TEXTURE_PATH, deviceHandler, commandBuffersHandler);
	descriptorSets = new DescriptorSetsHandler(logicalDevice, camera->getUniformBuffers(), texture);

	graphicsPipelineHandler = new GraphicsPipelineHandler(logicalDevice, swapchainHandler, descriptorSets->getDescriptorSetLayout(), renderPassHandler->getRenderPass());

	createSyncObjects();

	std::cout << "Vulkan successfully initialized.\n";
}

void Renderer::initImGui()
{
#ifdef DEBUG
	IMGUI_CHECKVERSION();
	ImGuiContext = ImGui::CreateContext();

	ImGui_ImplGlfw_InitForVulkan(windowHandler->getWindowPointer(), true);
	
	ImGui_ImplVulkan_InitInfo initInfo = {}; //taking after vulkan I see...
	initInfo.Instance = instanceHandler->getInstance();
	initInfo.PhysicalDevice = deviceHandler->getPhysicalDevice();
	initInfo.Device = deviceHandler->getLogicalDevice();
	initInfo.QueueFamily = deviceHandler->getQueueFamilyIndices().graphicsFamily.value();
	initInfo.Queue = deviceHandler->getGraphicsQueue();
	initInfo.PipelineCache = VK_NULL_HANDLE;
	initInfo.RenderPass = renderPassHandler->getRenderPass();
	initInfo.DescriptorPool = descriptorSets->getDescriptorPool();
	initInfo.Subpass = 0;
	initInfo.MinImageCount = MAX_FRAMES_IN_FLIGHT;
	initInfo.ImageCount = MAX_FRAMES_IN_FLIGHT;
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	initInfo.CheckVkResultFn = 0;

	ImGui_ImplVulkan_Init(&initInfo);

	ImGui_ImplVulkan_CreateFontsTexture();

	std::cout << "ImGui successfully initialized\n";
#endif
}

void Renderer::cleanup()
{
#ifdef DEBUG
	ImGui_ImplVulkan_DestroyFontsTexture();
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
#endif

	VkDevice& device = deviceHandler->getLogicalDevice();

	delete swapchainHandler;
	delete graphicsPipelineHandler;
	delete renderPassHandler;
	delete camera;
	delete descriptorSets;
	delete texture;

	scene->TerminateScene();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	delete commandBuffersHandler;
	delete deviceHandler;
	delete surfaceHandler; //surface must be deleted before the instance
	delete instanceHandler;
	delete windowHandler;

	glfwTerminate();

	std::cout << "Renderer successfully terminated.\n";
}

void Renderer::createSyncObjects() {
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; //so the first frame isn't waiting for a previous (nonexistant) frame to finish rendering

	VkDevice& device = deviceHandler->getLogicalDevice();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		if (
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS
			) throw std::runtime_error("Failed to create semaphores.\n");
	}

}

void Renderer::drawFrame() {
	VkDevice& device = deviceHandler->getLogicalDevice();

	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swapchainHandler->getSwapchain(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		swapchainHandler->recreateSwapchain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	processInput(windowHandler->getWindowPointer());
	camera->Update(currentFrame); //updates UBOs

	recordCommandBuffer(commandBuffersHandler->GetCommandBuffers()[currentFrame], imageIndex);

	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	//vkResetCommandBuffer(commandBuffersHandler->GetCommandBuffers()[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffersHandler->GetCommandBuffers()[currentFrame];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(deviceHandler->getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
		throw std::runtime_error("failed to submit draw command buffer!");

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapchainHandler->getSwapchain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(deviceHandler->getPresentQueue(), &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
		framebufferResized = false;
		swapchainHandler->recreateSwapchain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//optional
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) throw std::runtime_error("Failed to beign recording command buffer.\n");

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPassHandler->getRenderPass();
	renderPassInfo.framebuffer = swapchainHandler->getSwapchainFramebuffers()[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapchainHandler->getSwapchainExtent();

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0] = { {0.0f, 0.0f, 0.0f, 1.0f} }; //color attachments
	clearValues[1] = { 1.0f, 0 }; //default the depth values at each pixel to the farthest depth away (far plane)

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	//to commandBuffer, record a BeginRenderPass command, using &renderPassInfo, into a primary command buffer
	//all vkCmd functions return void; error handling is done after recording
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineHandler->getGraphicsPipeline());

	VkBuffer vertexBuffers[] = { scene->GetRenderInfo().vertexBuffer};
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, scene->GetRenderInfo().indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	//these are the dynamic state things specified when creating the pipeline:
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapchainHandler->getSwapchainExtent().width);
	viewport.height = static_cast<float>(swapchainHandler->getSwapchainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapchainHandler->getSwapchainExtent();
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineHandler->getPipelineLayout(), 0, 1, &descriptorSets->getDescriptorSets()[currentFrame], 0, nullptr);
	vkCmdDrawIndexed(commandBuffer, scene->GetRenderInfo().numIndices, 1, 0, 0, 0);

#ifdef DEBUG
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	processImGui();

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer, 0);
#endif

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) throw std::runtime_error("Failed to record command buffer!\n");
}