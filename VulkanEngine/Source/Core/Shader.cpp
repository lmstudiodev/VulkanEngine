#include <Core/Shader.h>
#include <Game/Game.h>
#include <Core/Device.h>

Shader::Shader(Game* game, const std::string& filename, bool debugMode) : m_game(game), m_debugMode(debugMode)
{
    auto shaderCode = readFile(filename);
    createShaderModule(shaderCode);
}

Shader::~Shader()
{
    if (m_debugMode)
        VKINFO("Shader class released");
}

void Shader::destroyShaderModule()
{
    if (m_debugMode)
        VKINFO("Shader module destroyed");
    
    vkDestroyShaderModule(m_game->m_device->getDevice(), m_shaderModule, nullptr);
}

VkShaderModule& Shader::getShaderModule()
{
    return m_shaderModule;
}

void Shader::createShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkResult result = vkCreateShaderModule(m_game->m_device->getDevice(), &createInfo, nullptr, &m_shaderModule);

    if (m_debugMode)
    {
        if (result != VK_SUCCESS)
            VKERROR_AND_THROW("Failed to create shader module!");

        VKINFO("Shader module created");
    }
}

std::vector<char> Shader::readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        if(m_debugMode)
            VKERROR_AND_THROW("Unable to open shader file: " << filename);
    }
        
    if (m_debugMode)
        VKINFO("Shader file " << filename << " loaded");

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}
