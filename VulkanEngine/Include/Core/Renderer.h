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
    VkSemaphore m_imageAvailableSemaphore;
    VkSemaphore m_renderFinishedSemaphore;
    VkFence m_inFlightFence;

    Game* m_game = nullptr;

    bool m_debugMode;
};

