#include <Core/Renderer.h>
#include <Core/Device.h>
#include <Core/SwapChain.h>
#include <Core/GraphicPipeline.h>

Renderer::Renderer(Game* game, bool debugMode) : m_game(game), m_debugMode(debugMode)
{
	createSyncObjects();
}

Renderer::~Renderer()
{
    if (m_debugMode)
        VKINFO("Renderer calss rleased");
}

void Renderer::cleanUp()
{
    for (size_t i = 0; i < m_game->m_swapChain->getSize(); i++)
    {
        vkDestroySemaphore(m_game->m_device->getDevice(), m_imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(m_game->m_device->getDevice(), m_renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(m_game->m_device->getDevice(), m_inFlightFences[i], nullptr);
    }
}

void Renderer::render()
{
    vkWaitForFences(m_game->m_device->getDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    vkResetFences(m_game->m_device->getDevice(), 1, &m_inFlightFences[m_currentFrame]);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(m_game->m_device->getDevice(), m_game->m_swapChain->getSwapChain(), UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

    m_game->m_device->resetCommandBuffer(m_currentFrame);

    recordCommandBuffer(m_game->m_device->getCommandBuffer(m_currentFrame), imageIndex);

    VkCommandBuffer cmdBuffer = m_game->m_device->getCommandBuffer(m_currentFrame);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame]};
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VkResult result = vkQueueSubmit(m_game->m_device->getGraphicQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]);

    if (result != VK_SUCCESS)
        VKERROR_AND_THROW("failed to submit draw command buffer!");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { m_game->m_swapChain->getSwapChain() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(m_game->m_device->getPresentationQueue(), &presentInfo);

    m_currentFrame = (m_currentFrame + 1) % m_game->m_swapChain->getSize();
}

void Renderer::createSyncObjects()
{
    m_imageAvailableSemaphores.resize(m_game->m_swapChain->getSize());
    m_renderFinishedSemaphores.resize(m_game->m_swapChain->getSize());
    m_inFlightFences.resize(m_game->m_swapChain->getSize());

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < m_game->m_swapChain->getSize(); i++)
    {
        if (vkCreateSemaphore(m_game->m_device->getDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_game->m_device->getDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_game->m_device->getDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
        {
            VKERROR_AND_THROW("failed to create semaphores!");
        }

        if (m_debugMode)
        {
            VKINFO("Semaphores created");
        }
    }
}

void Renderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);

    if (result != VK_SUCCESS)
        VKERROR_AND_THROW("failed to begin recording command buffer!");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_game->m_graphicPipeline->getRenderPass();
    renderPassInfo.framebuffer = m_game->m_swapChain->getFramBufferAtIndex(imageIndex);
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_game->m_swapChain->getExtent();

    VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_game->m_graphicPipeline->getGraphicPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_game->m_swapChain->getWidth());
    viewport.height = static_cast<float>(m_game->m_swapChain->getHeight());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = m_game->m_swapChain->getExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    result = vkEndCommandBuffer(commandBuffer);

    if (result != VK_SUCCESS)
        VKERROR_AND_THROW("Failed to record command buffer!");
}