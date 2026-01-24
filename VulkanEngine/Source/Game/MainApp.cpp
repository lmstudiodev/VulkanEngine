#include <Game/MainApp.h>

#include <stdexcept>
#include <array>

MainApp::MainApp()
{
	loadModels();
	createPipelineLayout();
	recreateSwapchain();
	createCommandBuffers();
}

MainApp::~MainApp()
{
	vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
}

void MainApp::loadModels()
{
	std::vector<LMVModel::Vertex> triangle_vertices{
		{{0.0, -0.5}, {1.0, 0, 0}},
		{{0.5, 0.5}, {0, 1.0, 0}},
		{{-0.5, 0.5}, {0, 0, 1.0}}
	};

	std::vector<LMVModel::Vertex> quad_vertices{
	{{-0.5, -0.5}, {1.0, 0, 0}},
	{{-0.5, 0.5}, {0, 1.0, 0}},
	{{0.5, 0.5}, {0, 0, 1.0}},
	{{-0.5, -0.5}, {1.0, 0, 0}},
	{{0.5, 0.5}, {0, 0, 1.0}},
	{{0.5, -0.5}, {1.0, 1.0, 0}}
	};

	m_model = std::make_unique<LMVModel>(m_device, triangle_vertices);
}

void MainApp::createPipelineLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Pipeline layout");
	}
}

void MainApp::createPipeline()
{
	assert(m_swapchain != nullptr && "Cannot create pipeline before swap chain");
	assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};
	
	LMVPipeline::defaultPipelineConfigInfo(pipelineConfig);
	pipelineConfig.renderPass = m_swapchain->getRenderPass();
	pipelineConfig.pipelineLayout = m_pipelineLayout;

	m_pipeline = std::make_unique<LMVPipeline>(m_device, "Resources/Shaders/vert.spv", "Resources/Shaders/frag.spv", pipelineConfig);
}

void MainApp::recreateSwapchain()
{
	auto extent = m_window.getExtent();

	while (extent.width == 0 || extent.height == 0)
	{
		extent = m_window.getExtent();
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(m_device.device());

	if (m_swapchain == nullptr)
	{
		m_swapchain = std::make_unique<LMVSwapChain>(m_device, extent);
	}
	else
	{
		m_swapchain = std::make_unique<LMVSwapChain>(m_device, extent, std::move(m_swapchain));

		if (m_swapchain->imageCount() != m_commandBuffers.size())
		{
			freeCommandBuffers();
			createCommandBuffers();
		}
	}

	createPipeline();
}

void MainApp::createCommandBuffers()
{
	m_commandBuffers.resize(m_swapchain->imageCount());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_device.getCommandPool();
	allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

	if (vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Falied to allocate command buffers");
	}
}

void MainApp::freeCommandBuffers()
{
	vkFreeCommandBuffers(m_device.device(), m_device.getCommandPool(), static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
	m_commandBuffers.clear();
}

void MainApp::drawFrame()
{
	uint32_t imageIndex;

	auto result = m_swapchain->acquireNextImage(&imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapchain();
		return;
	}

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain iamge");
	}

	recordCommandBuffer(imageIndex);

	result = m_swapchain->submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.wasWindowResized())
	{
		m_window.resetWindowResizedFlag();
		recreateSwapchain();
		return;
	}

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present swap chain");
	}
}

void MainApp::recordCommandBuffer(int imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vkQueueWaitIdle(m_device.graphicsQueue());

	vkResetCommandBuffer(m_commandBuffers[imageIndex], 0);

	if (vkBeginCommandBuffer(m_commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to begin recording command buffer");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_swapchain->getRenderPass();
	renderPassInfo.framebuffer = m_swapchain->getFrameBuffer(imageIndex);

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_swapchain->getSwapChainExtent();

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(m_commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_swapchain->getSwapChainExtent().width);
	viewport.height = static_cast<float>(m_swapchain->getSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor{ {0, 0}, m_swapchain->getSwapChainExtent() };
	vkCmdSetViewport(m_commandBuffers[imageIndex], 0, 1, &viewport);
	vkCmdSetScissor(m_commandBuffers[imageIndex], 0, 1, &scissor);

	m_pipeline->bind(m_commandBuffers[imageIndex]);
	m_model->bind(m_commandBuffers[imageIndex]);
	m_model->draw(m_commandBuffers[imageIndex]);

	vkCmdEndRenderPass(m_commandBuffers[imageIndex]);

	if (vkEndCommandBuffer(m_commandBuffers[imageIndex]) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record command buffer");
	}
}

void MainApp::run()
{
	while (!m_window.shouldClose())
	{
		glfwPollEvents();

		drawFrame();
	}

	vkDeviceWaitIdle(m_device.device());
}