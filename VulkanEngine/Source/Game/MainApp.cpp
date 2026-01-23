#include <Game/MainApp.h>

#include <stdexcept>
#include <array>

MainApp::MainApp()
{
	loadModels();
	createPipelineLayout();
	createPipeline();
	createCommandBuffers();
}

MainApp::~MainApp()
{
	vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
}

void MainApp::loadModels()
{
	std::vector<LMVModel::Vertex> vertices{
		{{0.0, -0.5}},
		{{0.5, 0.5}},
		{{-0.5, 0.5}}
	};

	m_model = std::make_unique<LMVModel>(m_device, vertices);
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
	auto pipelineConfig = LMVPipeline::defaultPipelineConfiInfo(m_swapchain.width(), m_swapchain.height());
	pipelineConfig.renderPass = m_swapchain.getRenderPass();
	pipelineConfig.pipelineLayout = m_pipelineLayout;

	m_pipeline = std::make_unique<LMVPipeline>(m_device, "Resources/Shaders/vert.spv", "Resources/Shaders/frag.spv", pipelineConfig);
}

void MainApp::createCommandBuffers()
{
	m_commandBuffers.resize(m_swapchain.imageCount());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_device.getCommandPool();
	allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

	if (vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Falied to allocate command buffers");
	}

	for (int i = 0; i < m_commandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;


		if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to begin recording command buffer");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_swapchain.getRenderPass();
		renderPassInfo.framebuffer = m_swapchain.getFrameBuffer(i);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_swapchain.getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		m_pipeline->bind(m_commandBuffers[i]);
		m_model->bind(m_commandBuffers[i]);
		m_model->draw(m_commandBuffers[i]);

		vkCmdEndRenderPass(m_commandBuffers[i]);

		if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to record command buffer");
		}
	}
}

void MainApp::drawFrame()
{
	uint32_t imageIndex;

	auto result = m_swapchain.acquireNextImage(&imageIndex);

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain iamge");
	}

	result = m_swapchain.submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present swap chain");
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