#include <Core/Surface.h>
#include <Game/Game.h>

Surface::Surface(Game* game, bool debugMode) : m_game(game), m_debugMode(debugMode)
{
    createSurface();
}

Surface::~Surface()
{
    if (m_debugMode)
        VKINFO("Surface class released");
}

VkSurfaceKHR Surface::getSurface()
{
    return m_surface;
}

void Surface::createSurface()
{
    VkResult result = glfwCreateWindowSurface(m_game->m_vkInstance, m_game->m_window, nullptr, &m_surface);

    if (result != VK_SUCCESS)
        VKERROR_AND_THROW("Unable to create window surface");

    VKINFO("Window surface created");
}