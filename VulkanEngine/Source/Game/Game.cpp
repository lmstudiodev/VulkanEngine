#include <Game/Game.h>
#include <Core/Device.h>
#include <Core/Surface.h>
#include <Core/SwapChain.h>
#include <Core/Shader.h>
#include <Core/GraphicPipeline.h>
#include <Core/Renderer.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    VKDEBUG(pCallbackData->pMessage);

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

    m_renderer->cleanUp();

    m_device->destroyCommandPool();

    m_swapChain->destroyFrameBuffer();

    m_graphicPipeline->cleanUp();

    m_swapChain->cleanUp();

    m_device->cleanUp();

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

    if(!m_window)
        VKERROR_AND_THROW("Window creation failed");

    if (m_debugMode)
        VKINFO("Window created successfully");
}

void Game::initVulkan()
{
    if (m_debugMode)
        VKINFO("Init Vulkan");
    
    createInstance();

    setupDebugMessenger();

    m_surface = std::make_unique<Surface>(this, m_debugMode);
    m_device = std::make_unique<Device>(this, m_debugMode);
    m_swapChain = std::make_unique<SwapChain>(this, m_debugMode);

    m_vertexShader = std::make_unique<Shader>(this, "Resources/Shaders/vert.spv", m_debugMode);
    m_fragmentShader = std::make_unique<Shader>(this, "Resources/Shaders/frag.spv", m_debugMode);

    m_graphicPipeline = std::make_unique<GraphicPipeline>(this, m_debugMode);
    m_graphicPipeline->setVertexShader(m_vertexShader.get());
    m_graphicPipeline->setFragmentShader(m_fragmentShader.get());
    m_graphicPipeline->init();

    m_swapChain->createFrameBuffer();
    m_device->createCommandPool();
    m_device->createCommandBuffer();

    m_renderer = std::make_unique<Renderer>(this, m_debugMode);
}

void Game::createInstance()
{
    if (m_debugMode)
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
        VKERROR_AND_THROW("Requested extensions not supported.");

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
            VKERROR_AND_THROW("Requested layers not supported.");
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

    if (result != VK_SUCCESS)
        VKERROR_AND_THROW("Failed to create Vulkan instance");

    if (m_debugMode)
        VKINFO("Vulkan instance created successfully");
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

    if (m_debugMode)
    {
        VKINFO("Extensions supported:");

        for (const auto& ext : _extensions)
        {
            VKINFO('\t' << ext.extensionName);
        }
    }

    for (auto extRequired : extensions)
    {
        bool found = false;

        for (const VkExtensionProperties& extSupported : _extensions)
        {
            if (strcmp(extRequired, extSupported.extensionName) == 0)
            {
                found = true;

                if (m_debugMode)
                    VKINFO("Extension " << extRequired << " supported");
            }
        }

        if (!found)
        {
            if (m_debugMode)
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
        VKERROR_AND_THROW("Failed to set up debug messenger!");

    VKINFO("Debug messenger created");
}

void Game::drawFrame()
{
    m_renderer->render();
}
