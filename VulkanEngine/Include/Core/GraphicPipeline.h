#pragma once
#include <Prerequisite.h>
#include <Game/Game.h>

class GraphicPipeline
{
public:
    GraphicPipeline(Game* game, bool debugMode);
    ~GraphicPipeline();

public:
    void init();
    void setVertexShader(Shader* vertexShader);
    void setFragmentShader(Shader* fragmentShader);
    void createRenderPass();
    void cleanUp();
    VkPipelineLayout getLayout();
    VkPipeline getGraphicPipeline();
    VkRenderPass getRenderPass();

private:
    VkPipelineShaderStageCreateInfo m_vertShaderStageInfo{};
    VkPipelineShaderStageCreateInfo m_fragShaderStageInfo{};
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_graphicsPipeline;
    VkRenderPass m_renderPass;

    Shader* m_vertexShader;
    Shader* m_fragmentShader;

    Game* m_game = nullptr;

    bool m_debugMode;
};

