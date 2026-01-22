#include <Game/Game.h>
#include <Core/Device.h>
#include <Core/Surface.h>
#include <Core/SwapChain.h>
#include <Core/Shader.h>
#include <Core/GraphicPipeline.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    //if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) 
    //{
        VKDEBUG(pCallbackData->pMessage);
    //}

    return VK_FALSE;
}

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, 
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
    const VkAllocationCallbacks* pAllocator, 
    VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance, 
    VkDebugUtilsMessengerEXT debugMessenger, 
    const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

Game::Game() : m_window(nullptr), m_vkInstance(nullptr)
{
    if (m_debugMode)
        VKINFO("Initialize Game");
    
    initWindow();
    initVulkan();
}

Game::~Game()
{
    cleanup();
}

void Game::run()
{
    mainLoop();
}

void Game::mainLoop()
{
    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
        this->drawFrame();
    }

    vkDeviceWaitIdle(m_device->getDevice());
}

void Game::cleanup()
{
    if (m_debugMode)
        VKINFO("Engine shutdown");

    vkDestroySemaphore(m_device->getDevice(), m_imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(m_device->getDevice(), m_renderFinishedSemaphore, nullptr);
    vkDestroyFence(m_device->getDevice(), m_inFlightFence, nullptr);

    vkDestroyCommandPool(m_device->getDevice(), m_commandPool, nullptr);

    for (auto framebuffer : m_swapChainFramebuffers)
    {
        vkDestroyFramebuffer(m_device->getDevice(), framebuffer, nullptr);
    }

    vkDestroyPipeline(m_device->getDevice(), m_graphicPipeline->getGraphicPipeline(), nullptr);

    vkDestroyPipelineLayout(m_device->getDevice(), m_graphicPipeline->getLayout(), nullptr);

    vkDestroyRenderPass(m_device->getDevice(), m_graphicPipeline->getRenderPass(), nullptr);

    m_swapChain->cleanUp();

    vkDestroySwapchainKHR(m_device->getDevice(), m_swapChain->getSwapChain(), nullptr);

    vkDestroyDevice(m_device->getDevice(), nullptr);

    if (m_debugMode)
    {
        DestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(m_vkInstance, m_surface->getSurface(), nullptr);
    vkDestroyInstance(m_vkInstance, nullptr);
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Game::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

    if (m_debugMode)
    {
        if (m_window)
        {
            VKINFO("Window created successfully");
        }
        else
        {
            VKERROR_AND_THROW("Window creation failed");
        }
    }
}

void Game::initVulkan()
{
    VKINFO("Init Vulkan");
    
    createInstance();

    setupDebugMessenger();

    m_surface = std::make_unique<Surface>(this, m_debugMode);
    m_device = std::make_unique<Device>(this, m_debugMode);
    m_swapChain = std::make_unique<SwapChain>(this, m_debugMode);

    createGraphicsPipeline();

    createFramebuffers();

    createCommandPool();

    createCommandBuffer();

    createSyncObjects();
}

void Game::createInstance()
{
    VKINFO("Create instance");

    uint32_t version{ 0 };
    vkEnumerateInstanceVersion(&version);

    if (m_debugMode)
    {
        VKINFO("System can support Vulkan variant " << VK_API_VERSION_VARIANT(version));
        VKINFO("Major " << VK_API_VERSION_MAJOR(version));
        VKINFO("Minor " << VK_API_VERSION_MINOR(version));
        VKINFO("Patch " << VK_API_VERSION_PATCH(version));
    }

    version &= ~(0xFFFU);
    version = VK_MAKE_API_VERSION(0, 1, 0, 0);

    VkApplicationInfo appInfo{};
    appInfo.pApplicationName = "VulkanEngine";
    appInfo.pEngineName = "LMEngine";
    appInfo.applicationVersion = version;
    appInfo.engineVersion = version;
    appInfo.apiVersion = version;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (m_debugMode)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        
        VKINFO("Extensions requested:");

        for (const char* extensionName : extensions)
        {
            VKINFO('\t' << extensionName);
        }
    }

    if(!checkForSupportedExtensions(extensions))
    {
        VKERROR_AND_THROW("Requested extensions not supported.");
    }

    std::vector<const char*> layers{};

    if (m_debugMode)
    {
        VKINFO("Layers requested:");

        layers.push_back("VK_LAYER_KHRONOS_validation");

        for (const char* layersName : layers)
        {
            VKINFO('\t' << layersName);
        }

        if (!checkForSupportedLayers(layers))
        {
            VKERROR_AND_THROW("Requested layers not supported.");
        }
    }

    VkInstanceCreateFlags flags{};

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.flags = flags,
    createInfo.pApplicationInfo = &appInfo;

    if (m_debugMode)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugCallback;
        debugCreateInfo.pUserData = nullptr;
        
        createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        createInfo.ppEnabledLayerNames = layers.data();
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_vkInstance);

    if (m_debugMode)
    {
        if (result != VK_SUCCESS)
        {
            VKERROR_AND_THROW("Failed to create Vulkan instance");
        }
        else
        {
            VKINFO("Vulkan instance created successfully");
        }
    }
}

bool Game::checkForSupportedExtensions(std::vector<const char*>& extensions) const
{    
    uint32_t extensionCount = 0;
    VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    if (m_debugMode)
    {
        if (result != VK_SUCCESS)
            VKERROR_AND_THROW("Failed to enumerate extension properties");
    }

    std::vector<VkExtensionProperties> _extensions(extensionCount);

    result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, _extensions.data());

    if (m_debugMode)
    {
        if (result != VK_SUCCESS)
            VKERROR_AND_THROW("Failed to enumerate extension properties");
    }

    VKINFO("Extensions supported:");

    for (const auto& ext : _extensions) 
    {
        VKINFO('\t' << ext.extensionName);
    }

    for (auto extRequired : extensions)
    {
        bool found = false;

        for (const VkExtensionProperties& extSupported : _extensions)
        {
            if (strcmp(extRequired, extSupported.extensionName) == 0)
            {
                found = true;

                VKINFO("Extension " << extRequired << " supported");
            }
        }

        if (!found)
        {
            VKERROR("One or more requested extensions not found");
            return false;
        }
    }

    return true;
}

bool Game::checkForSupportedLayers(std::vector<const char*>& layers) const
{
    uint32_t layerCount;
    VkResult result = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    if (m_debugMode)
    {
        if (result != VK_SUCCESS)
            VKERROR_AND_THROW("Failed to enumerate layers properties");
    }

    std::vector<VkLayerProperties> availableLayers(layerCount);
    result = vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    if (m_debugMode)
    {
        if (result != VK_SUCCESS)
            VKERROR_AND_THROW("Failed to enumerate layers properties");
    }

    VKINFO("Layers supported:");

    for (const auto& layer : availableLayers)
    {
        VKINFO('\t' << layer.layerName);
    }

    for (auto layerRequired : layers)
    {
        bool found = false;
        
        for (const VkLayerProperties& layerSupported : availableLayers)
        {
            if (strcmp(layerRequired, layerSupported.layerName) == 0)
            {
                found = true;

                VKINFO("Layer " << layerRequired << " supported");
            }
        }

        if (!found)
        {
            VKERROR("One or more requested layers not found");
            return false;
        }
    }
    
    return true;
}

void Game::setupDebugMessenger()
{
    if (!m_debugMode)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;

    if (CreateDebugUtilsMessengerEXT(m_vkInstance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
    {
        VKERROR_AND_THROW("Failed to set up debug messenger!");
    }
}

void Game::createGraphicsPipeline()
{
    m_vertexShader = std::make_unique<Shader>(this, "Resources/Shaders/vert.spv", m_debugMode);
    m_fragmentShader = std::make_unique<Shader>(this, "Resources/Shaders/frag.spv", m_debugMode);

    m_graphicPipeline = std::make_unique<GraphicPipeline>(this, m_debugMode);
    m_graphicPipeline->setVertexShader(m_vertexShader.get());
    m_graphicPipeline->setFragmentShader(m_fragmentShader.get());
    m_graphicPipeline->init();
}

void Game::createFramebuffers()
{    
    size_t swapChainSize = m_swapChain->getSize();
    
    m_swapChainFramebuffers.resize(swapChainSize);

    for (size_t i = 0; i < swapChainSize; i++)
    {
        VkImageView attachments[] = {
            m_swapChain->getImageView(i)
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_graphicPipeline->getRenderPass();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_swapChain->getWidth(); //    swapChainExtent.width;
        framebufferInfo.height = m_swapChain->getHeight(); //  swapChainExtent.height;
        framebufferInfo.layers = 1;

        VkResult result = vkCreateFramebuffer(m_device->getDevice(), &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]);

        if (m_debugMode)
        {
            if (result != VK_SUCCESS)
                VKERROR_AND_THROW("failed to create framebuffer!");

            VKINFO("Frame buffer " << i << " created");
        }
    }
}

void Game::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = m_device->getQueueFamilyIndices();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    VkResult result = vkCreateCommandPool(m_device->getDevice(), &poolInfo, nullptr, &m_commandPool);

    if (m_debugMode)
    {
        if (result != VK_SUCCESS)
            VKERROR_AND_THROW("Failed to create command pool!");

        VKINFO("Command pool created");
    }
}

void Game::createCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkResult result = vkAllocateCommandBuffers(m_device->getDevice(), &allocInfo, &m_commandBuffer);

    if (m_debugMode)
    {
        if (result != VK_SUCCESS)
            VKERROR_AND_THROW("failed to allocate command buffers!");

        VKINFO("Command buffer created");
    }
}

void Game::createSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(m_device->getDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(m_device->getDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(m_device->getDevice(), &fenceInfo, nullptr, &m_inFlightFence) != VK_SUCCESS) 
    {
        if (m_debugMode)
        {
            VKERROR_AND_THROW("failed to create semaphores!");
        }
    }

    if (m_debugMode)
    {
        VKINFO("Semaphores created");
    }
}

void Game::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    
    VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    if (m_debugMode)
    {
        if (result != VK_SUCCESS)
            VKERROR_AND_THROW("failed to begin recording command buffer!");

        VKINFO("Command buffer begin recording");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_graphicPipeline->getRenderPass();
    renderPassInfo.framebuffer = m_swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_swapChain->getExtent();

    VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicPipeline->getGraphicPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapChain->getWidth());
    viewport.height = static_cast<float>(m_swapChain->getHeight());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = m_swapChain->getExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    result = vkEndCommandBuffer(commandBuffer);

    if (m_debugMode)
    {
        if (result != VK_SUCCESS)
            VKERROR_AND_THROW("Failed to record command buffer!");

        VKINFO("Command buffer recorded");
    }
}

void Game::drawFrame()
{
    vkWaitForFences(m_device->getDevice(), 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);

    vkResetFences(m_device->getDevice(), 1, &m_inFlightFence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(m_device->getDevice(), m_swapChain->getSwapChain(), UINT64_MAX, m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    vkResetCommandBuffer(m_commandBuffer, 0);

    recordCommandBuffer(m_commandBuffer, imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer;

    VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;


    VkResult result = vkQueueSubmit(m_device->getGraphicQueue(), 1, &submitInfo, m_inFlightFence);

    if (m_debugMode)
    {
        if (result != VK_SUCCESS)
        {
            VKERROR_AND_THROW("failed to submit draw command buffer!");
        }

        VKINFO("draw command buffer submitted");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { m_swapChain->getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(m_device->getPresentationQueue(), &presentInfo);
}
