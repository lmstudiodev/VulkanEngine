#pragma once
#include <Prerequisite.h>
#include <Game/Game.h>
#include <vector>

class Shader
{
public:
    Shader(Game* game, const std::string& filename, bool debugMode);
    ~Shader();

public:
    void destroyShaderModule();
    VkShaderModule& getShaderModule();

private:
    std::vector<char> readFile(const std::string& filename);
    void createShaderModule(const std::vector<char>& code);

private:
    VkShaderModule m_shaderModule;

    Game* m_game;

    bool m_debugMode;
};

