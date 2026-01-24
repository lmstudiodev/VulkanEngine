#include <Window/LMVWindow.h>
#include <stdexcept>

LMVWindow::LMVWindow(int w, int h, std::string wName) : m_width(w), m_height(h), m_windowName(wName), m_window(nullptr)
{
	initWindow();
}

LMVWindow::~LMVWindow()
{
    glfwDestroyWindow(m_window);

    glfwTerminate();
}

void LMVWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
{
    if (glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface");
    }
}

void LMVWindow::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(m_width, m_height, m_windowName.c_str(), nullptr, nullptr);

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, frameBufferResizeCallback);
}

void LMVWindow::frameBufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto classWindow = reinterpret_cast<LMVWindow*>(glfwGetWindowUserPointer(window));

    classWindow->m_frameBufferResized = true;
    classWindow->m_width = width;
    classWindow->m_height = height;
}
