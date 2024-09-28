#pragma once

#include "Components/TransformComponent.h"
#include "Components/VoxelModel.h"
#include "Voxel.h"
#include "src/vulkanHandlers/DeviceHandler.h"

struct RendererInfo
{
	DeviceHandler* deviceHandler;
	CommandBuffersHandler* commandBuffersHandler;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	uint32_t numIndices = 0;
};

class Scene
{
	std::unordered_map<uint32_t, Voxel> voxels;
	uint32_t idCount = 0;
	RendererInfo ri;

public:
	Scene(DeviceHandler* _dh, CommandBuffersHandler* _cbh)
	{
		ri.deviceHandler = _dh;
		ri.commandBuffersHandler = _cbh;
	}

	RendererInfo& GetRenderInfo() { return ri; }

	void AddVoxel(glm::vec3 position)
	{
		Voxel v;
		v.Transform = TransformComponent(position);
		voxels[idCount] = v;
		++idCount;
	}

	void FinishScene()
	{
		createVertexBuffer();
		createIndexBuffer();
	}

	void TerminateScene()
	{
		VkDevice& device = ri.deviceHandler->getLogicalDevice();
		vkDestroyBuffer(device, ri.vertexBuffer, nullptr);
		vkFreeMemory(device, ri.vertexBufferMemory, nullptr);
		vkDestroyBuffer(device, ri.indexBuffer, nullptr);
		vkFreeMemory(device, ri.indexBufferMemory, nullptr);
	}

private:
	void createVertexBuffer() 
	{
		VkDeviceSize bufferSize = sizeof(Vertex) * voxels.size() * 8;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		BufferHelpers::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, ri.deviceHandler);

		VkDevice& device = ri.deviceHandler->getLogicalDevice();

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		
		int i = 0;
		for (auto& pair : voxels)
		{
			memcpy((Vertex*)data + sizeof(Vertex) * i * 8, pair.second.VoxelModel.getVertexData(), sizeof(Vertex) * 8);
			++i;
		}

		vkUnmapMemory(device, stagingBufferMemory);

		BufferHelpers::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ri.vertexBuffer, ri.vertexBufferMemory, ri.deviceHandler);

		BufferHelpers::CopyBuffer(stagingBuffer, ri.vertexBuffer, bufferSize, ri.commandBuffersHandler);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void createIndexBuffer() {
		VkDeviceSize bufferSize = 36 * voxels.size() * sizeof(uint32_t);

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		BufferHelpers::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, ri.deviceHandler);

		VkDevice& device = ri.deviceHandler->getLogicalDevice();

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);

		int i = 0;
		for (auto& pair : voxels)
		{
			memcpy((uint32_t*) data + 36 * sizeof(uint32_t) * i, pair.second.VoxelModel.getIndicesData(), 36 * sizeof(uint32_t));
			++i;
		}
		ri.numIndices = i * 36;

		vkUnmapMemory(device, stagingBufferMemory);

		BufferHelpers::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ri.indexBuffer, ri.indexBufferMemory, ri.deviceHandler);
		BufferHelpers::CopyBuffer(stagingBuffer, ri.indexBuffer, bufferSize, ri.commandBuffersHandler);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}
};