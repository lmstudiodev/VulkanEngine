#pragma once
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <Prerequisite.h>
#include <Game/Game.h>
#include <vector>
#include <optional>
#include <set>

class Device
{
public:
	Device(Game* game, bool debugMode);
	~Device();

public:
	VkPhysicalDevice getPhysicalDevice();
	VkDevice getDevice();
	VkQueue getGraphicQueue();
	VkQueue getPresentationQueue();
	VkCommandBuffer getCommandBuffer();
	SwapChainSupportDetails getSwapchainSupport();
	QueueFamilyIndices getQueueFamilyIndices();
	void createCommandPool();
	void createCommandBuffer();
	void cleanUp();
	void destroyCommandPool();
	void resetCommandBuffer();

private:
	void pickPhysicalDevice(VkInstance& instance);
	void createLogicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);


private:
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_device = nullptr;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;
	QueueFamilyIndices m_queueFamilyIndices;
	SwapChainSupportDetails m_swapChainSupport;
	VkCommandPool m_commandPool;
	VkCommandBuffer m_commandBuffer;

	Game* m_game = nullptr;

	bool m_debugMode;
};

