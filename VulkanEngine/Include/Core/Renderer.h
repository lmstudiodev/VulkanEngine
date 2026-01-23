#pragma once
#include <Prerequisite.h>
#include <Game/Game.h>

class Renderer
{
public:
    Renderer(Game* game, bool debugMode);
    ~Renderer();

public:
    void render();
    void cleanUp();

private:
    void createSyncObjects();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

private:
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;

    Game* m_game = nullptr;

    bool m_debugMode;

    uint32_t m_currentFrame = 0;
};

