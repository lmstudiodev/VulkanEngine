#include <Core/Device.h>
#include <Core/Surface.h>
#include <Game/Game.h>

Device::Device(Game* game, bool debugMode) : m_game(game), m_debugMode(debugMode)
{
    pickPhysicalDevice(game->m_vkInstance);
    createLogicalDevice();;
}

Device::~Device()
{
    if(m_debugMode)
        VKINFO("Device class released");
}

void Device::cleanUp()
{
    vkDestroyDevice(m_device, nullptr);
}

void Device::destroyCommandPool()
{
    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
}

void Device::resetCommandBuffer()
{
    vkResetCommandBuffer(m_commandBuffer, 0);
}

VkPhysicalDevice Device::getPhysicalDevice()
{
	return m_physicalDevice;
}

VkDevice Device::getDevice()
{
	return m_device;
}

VkQueue Device::getGraphicQueue()
{
    return m_graphicsQueue;
}

VkQueue Device::getPresentationQueue()
{
    return m_presentQueue;
}

VkCommandBuffer Device::getCommandBuffer()
{
    return m_commandBuffer;
}

SwapChainSupportDetails Device::getSwapchainSupport()
{
    return m_swapChainSupport;
}

QueueFamilyIndices Device::getQueueFamilyIndices()
{
    return m_queueFamilyIndices;
}

void Device::pickPhysicalDevice(VkInstance& instance)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
        VKERROR_AND_THROW("Failed to find GPUs with Vulkan support!");
    
    if (m_debugMode)
        VKINFO("Found " << deviceCount << " GPUs with Vulkan support");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    if (result != VK_SUCCESS)
        VKERROR_AND_THROW("Failed to enumerate GPUs");

    for (const auto& device : devices)
    {
        if (isDeviceSuitable(device))
        {
            m_physicalDevice = device;

            break;
        }
    }

    if (m_physicalDevice == VK_NULL_HANDLE)
    {
        VKERROR_AND_THROW("Failed to find a suitable GPU!");
    }
}

void Device::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
        VKERROR_AND_THROW("Failed to create logical device!");

    if (m_debugMode)
        VKINFO("Logical device created");

    vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
}

bool Device::isDeviceSuitable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader)
    {
        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;

        if (extensionsSupported)
        {
            m_swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !m_swapChainSupport.formats.empty() && !m_swapChainSupport.presentModes.empty();
        }

        if (indices.isComplete() && extensionsSupported && swapChainAdequate)
        {
            if (m_debugMode)
            {
                VKINFO("Found a suitable GPU!");
                VKINFO('\t' << deviceProperties.deviceName);
            }

            return true;
        }
    }

    return false;
}

QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices{};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;

    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_game->m_surface->getSurface(), &presentSupport);

        if (presentSupport)
        {
            indices.presentFamily = i;
        }

        i++;
    }

    m_queueFamilyIndices = indices;

    return indices;
}

bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

SwapChainSupportDetails Device::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_game->m_surface->getSurface(), &details.capabilities);

    if (result != VK_SUCCESS)
        VKERROR_AND_THROW("Unable to get device surface capabilities");

    uint32_t formatCount;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_game->m_surface->getSurface(), &formatCount, nullptr);

    if (result != VK_SUCCESS)
        VKERROR_AND_THROW("Unable to get device surface format");

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_game->m_surface->getSurface(), &formatCount, details.formats.data());

        if (result != VK_SUCCESS)
            VKERROR_AND_THROW("Unable to get device surface format");
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_game->m_surface->getSurface(), &presentModeCount, nullptr);

    if (result != VK_SUCCESS)
        VKERROR_AND_THROW("Unable to get device surface presentation modes");

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_game->m_surface->getSurface(), &presentModeCount, details.presentModes.data());

        if (result != VK_SUCCESS)
            VKERROR_AND_THROW("Unable to get device surface presentation modes");
    }

    return details;
}

void Device::createCommandPool()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_queueFamilyIndices.graphicsFamily.value();

    VkResult result = vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool);

    if (result != VK_SUCCESS)
        VKERROR_AND_THROW("Failed to create command pool!");

    if (m_debugMode)
        VKINFO("Command pool created");
}

void Device::createCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkResult result = vkAllocateCommandBuffers(m_device, &allocInfo, &m_commandBuffer);

    if (result != VK_SUCCESS)
        VKERROR_AND_THROW("failed to allocate command buffers!");

    if (m_debugMode)
        VKINFO("Command buffer created");
}