#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <Prerequisite.h>
#include <vector>
#include <optional>
#include <set>

struct QueueFamilyIndices 
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails 
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Game
{
public:
    Game();
    ~Game();

public:
    void run();

private:
    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();

private:
    void createInstance();
    void setupDebugMessenger();
    bool checkForSupportedExtensions(std::vector<const char*>& extensions) const;
    bool checkForSupportedLayers(std::vector<const char*>& layers) const;

    void drawFrame();

private:
    GLFWwindow* m_window;
    VkInstance m_vkInstance;
    VkDebugUtilsMessengerEXT m_debugMessenger;

    const uint32_t WIDTH = 1920;
    const uint32_t HEIGHT = 1080;

    std::shared_ptr<Device> m_device;
    std::shared_ptr<Surface> m_surface;
    std::shared_ptr<SwapChain> m_swapChain;

    std::shared_ptr<Shader> m_vertexShader;
    std::shared_ptr<Shader> m_fragmentShader;
    std::shared_ptr<GraphicPipeline> m_graphicPipeline;
    std::shared_ptr<Renderer> m_renderer;

private:
    friend class Device;
    friend class Surface;
    friend class SwapChain;
    friend class Shader;
    friend class GraphicPipeline;
    friend class Renderer;

#ifndef NDEBUG
    bool m_debugMode = true;
#else
    bool m_debugMode = false;
#endif

};

