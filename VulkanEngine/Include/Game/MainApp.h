#pragma once
#include <Window/LMVWindow.h>
#include <Core/LMVPipeline.h>
#include <Core/LMVDevice.h>
#include <Core/LMVSwapChain.h>
#include <Core/LMVModel.h>

#include <memory>
#include <vector>

class MainApp
{
public:
	MainApp();
	~MainApp();

	MainApp(const MainApp&) = delete;
	void operator = (const MainApp&) = delete;

public:
	void run();

private:
	void loadModels();
	void createPipelineLayout();
	void createPipeline();
	void createCommandBuffers();
	void drawFrame();

public:
	static constexpr int WIDTH = 1920;
	static constexpr int HEIGHT = 1080;

private:
	LMVWindow m_window{WIDTH, HEIGHT, "LMV Vulkan Engine"};
	LMVDevice m_device{m_window};
	LMVSwapChain m_swapchain{m_device, m_window.getExtent()};

	std::unique_ptr<LMVPipeline> m_pipeline;
	std::unique_ptr<LMVModel> m_model;

	VkPipelineLayout m_pipelineLayout;

	std::vector<VkCommandBuffer> m_commandBuffers;
};

