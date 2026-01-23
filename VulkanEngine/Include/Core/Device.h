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
	std::vector<VkCommandBuffer> getAllCommandBuffers();
	VkCommandBuffer getCommandBuffer(size_t index);
	SwapChainSupportDetails getSwapchainSupport();
	QueueFamilyIndices getQueueFamilyIndices();
	void createCommandPool();
	void createCommandBuffer();
	void cleanUp();
	void destroyCommandPool();
	void resetAllCommandBuffers();
	void resetCommandBuffer(size_t index);

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
	std::vector<VkCommandBuffer> m_commandBuffers;

	Game* m_game = nullptr;

	bool m_debugMode;
};

