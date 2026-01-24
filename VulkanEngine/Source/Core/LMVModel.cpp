#include <Core/LMVModel.h>

#include <cassert>
#include <cstring>

LMVModel::LMVModel(LMVDevice& device, const std::vector<Vertex>& vertices) : m_device(device)
{
	createVertexBuffer(vertices);
}

LMVModel::~LMVModel()
{
	vkDestroyBuffer(m_device.device(), m_buffer, nullptr);

	vkFreeMemory(m_device.device(), m_vertexBuffeerMemory, nullptr);
}

void LMVModel::createVertexBuffer(const std::vector<Vertex>& vertices)
{
	m_vertexCount = static_cast<uint32_t>(vertices.size());

	assert(m_vertexCount >= 3 && "Vertex count must be at least 3");

	VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;

	m_device.createBuffer(bufferSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		m_buffer,
		m_vertexBuffeerMemory
		);

	void* data;

	vkMapMemory(m_device.device(), m_vertexBuffeerMemory, 0, bufferSize, 0, &data);

	memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));

	vkUnmapMemory(m_device.device(), m_vertexBuffeerMemory);
}

void LMVModel::bind(VkCommandBuffer commandBuffer)
{
	VkBuffer buffers[] = { m_buffer };
	VkDeviceSize offsets[] = {0};

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void LMVModel::draw(VkCommandBuffer commandBuffer)
{
	vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
}

std::vector<VkVertexInputBindingDescription> LMVModel::Vertex::getBindingDescriptions()
{
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(Vertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	
	return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> LMVModel::Vertex::getAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, position);;

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, color);

	return attributeDescriptions;
}
