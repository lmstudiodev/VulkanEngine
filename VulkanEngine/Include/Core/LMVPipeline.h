#pragma once
#include <Core/LMVDevice.h>
#include <string>
#include <vector>

struct PipelineConfigInfo
{
	VkViewport viewport;
	VkRect2D scissor;
	//VkPipelineViewportStateCreateInfo viewportInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	VkPipelineRasterizationStateCreateInfo rasterizationInfo;
	VkPipelineMultisampleStateCreateInfo multisampleInfo;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo colorBlendInfo;
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
	VkPipelineLayout pipelineLayout = nullptr;
	VkRenderPass renderPass = nullptr;
	uint32_t subpass = 0;
};

class LMVPipeline
{
public:
	LMVPipeline(LMVDevice& device, const std::string& vertexShaderPath, const std::string& pixelShaderPath, const PipelineConfigInfo& configInfo);
	~LMVPipeline();

	LMVPipeline(const LMVPipeline&) = delete;
	void operator = (const LMVPipeline&) = delete;

	static PipelineConfigInfo defaultPipelineConfiInfo(uint32_t width, uint32_t height);

public:
	void bind(VkCommandBuffer commandBuffer);

private:
	static std::vector<char> readFile(const std::string& filePath);

	void createGraphicsPipeline(const std::string& vertexShaderPath, const std::string& pixelShaderPath, const PipelineConfigInfo& config);
	void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

private:
	LMVDevice& m_device;
	VkPipeline m_graphicsPipeline;
	VkShaderModule m_vertexShaderModule;
	VkShaderModule m_fragmentShaderModule;
};

