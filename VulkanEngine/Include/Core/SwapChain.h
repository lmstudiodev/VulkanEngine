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
	void destroyFrameBuffer();
	uint32_t getWidth();
	uint32_t getHeight();
	VkExtent2D& getExtent();
	VkFormat& getFormat();
	size_t getSize();
	VkImageView getImageView(size_t index);
	VkFramebuffer getFramBufferAtIndex(size_t index);
	std::vector<VkFramebuffer> getFrameBufferList();

	void createFrameBuffer();

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
	std::vector<VkFramebuffer> m_swapChainFramebuffers;
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;

	Game* m_game = nullptr;

	bool m_debugMode;
};

