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
		VkDeviceSize bufferSize = sizeof(Vertex) * VoxelModel::VERTICIES_PER_VOXEL * voxels.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		BufferHelpers::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, ri.deviceHandler);

		VkDevice& device = ri.deviceHandler->getLogicalDevice();

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		
		int voxelOn = 0;

		for (auto& IDVoxelPair : voxels)
		{
			Vertex* writeStart = (Vertex*)data + VoxelModel::VERTICIES_PER_VOXEL * voxelOn;
			Vertex* readStart = IDVoxelPair.second.VoxelModel.getVertexData();
			for (Vertex *writePtr = writeStart, *readPtr = readStart;
				 writePtr < writeStart + VoxelModel::VERTICIES_PER_VOXEL && readPtr < readStart + VoxelModel::VERTICIES_PER_VOXEL;
				++writePtr, ++readPtr)
			{
				*writePtr = *readPtr;

				glm::vec4 homogenousPos = glm::vec4(readPtr->pos, 1.0f);
				homogenousPos = IDVoxelPair.second.Transform.GetTransformMatrix() * homogenousPos;

				writePtr->pos.x = homogenousPos.x;
				writePtr->pos.y = homogenousPos.y;
				writePtr->pos.z = homogenousPos.z;
			}

			++voxelOn;
		}

		vkUnmapMemory(device, stagingBufferMemory);

		BufferHelpers::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ri.vertexBuffer, ri.vertexBufferMemory, ri.deviceHandler);

		BufferHelpers::CopyBuffer(stagingBuffer, ri.vertexBuffer, bufferSize, ri.commandBuffersHandler);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void createIndexBuffer() {
		VkDeviceSize bufferSize = sizeof(uint32_t) * VoxelModel::INDICES_PER_VOXEL * voxels.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		BufferHelpers::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, ri.deviceHandler);

		VkDevice& device = ri.deviceHandler->getLogicalDevice();

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);

		int voxelOn = 0;

		for (auto& IDVoxelPair : voxels)
		{
			uint32_t* writeStart = (uint32_t*)data + VoxelModel::INDICES_PER_VOXEL * voxelOn;
			uint32_t* readStart = IDVoxelPair.second.VoxelModel.getIndicesData();
			for (uint32_t* writePtr = writeStart, *readPtr = readStart;
				writePtr < writeStart + VoxelModel::INDICES_PER_VOXEL && readPtr < readStart + VoxelModel::INDICES_PER_VOXEL;
				++writePtr, ++readPtr)
			{
				*(writePtr) = *(readPtr) + VoxelModel::VERTICIES_PER_VOXEL * voxelOn;
			}

			++voxelOn;
		}
		ri.numIndices = voxelOn * 36;

		vkUnmapMemory(device, stagingBufferMemory);

		BufferHelpers::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ri.indexBuffer, ri.indexBufferMemory, ri.deviceHandler);
		BufferHelpers::CopyBuffer(stagingBuffer, ri.indexBuffer, bufferSize, ri.commandBuffersHandler);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}
};