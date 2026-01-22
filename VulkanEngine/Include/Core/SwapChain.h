#pragma once
#include <Prerequisite.h>
#include <Game/Game.h>

class SwapChain
{
public:
	SwapChain(Game* game, bool debugMode);
	~SwapChain();

public:
	VkSwapchainKHR getSwapChain();
	void cleanUp();
	float getWidth();
	float getHeight();
	VkExtent2D& getExtent();
	VkFormat& getFormat();
	size_t getSize();
	VkImageView getImageView(size_t index);

private:
	void createSwapChain();
	void createImageViews();

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

private:
	VkSwapchainKHR m_swapChain;
	std::vector<VkImage> m_swapChainImages;
	std::vector<VkImageView> m_swapChainImageViews;
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;

	Game* m_game = nullptr;

	bool m_debugMode;
};

