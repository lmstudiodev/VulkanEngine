#pragma once
#include <Core/LMVDevice.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

class LMVModel
{
public:
	struct Vertex
	{
		glm::vec2 position;
		glm::vec3 color;

		static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
	};


public:
	LMVModel(LMVDevice& device, const std::vector<Vertex>& vertices);
	~LMVModel();

	LMVModel(const LMVModel&) = delete;
	void operator=(const LMVModel&) = delete;

private:
	void createVertexBuffer(const std::vector<Vertex>& vertices);

public:
	void bind(VkCommandBuffer commandBuffer);
	void draw(VkCommandBuffer commandBuffer);

private:
	LMVDevice& m_device;

	VkBuffer m_buffer;
	VkDeviceMemory m_vertexBuffeerMemory;
	uint32_t m_vertexCount;
};

