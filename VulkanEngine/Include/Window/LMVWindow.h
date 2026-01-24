#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

class LMVWindow
{
public:
	LMVWindow(int w, int h, std::string wName);
	~LMVWindow();

	LMVWindow(const LMVWindow &) = delete;
	LMVWindow &operator = (const LMVWindow &) = delete;

public:
	inline bool shouldClose() { return glfwWindowShouldClose(m_window); }
	bool wasWindowResized() { return m_frameBufferResized; }
	void resetWindowResizedFlag() { m_frameBufferResized = false; }

	VkExtent2D getExtent() { return { static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height) }; }

	void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

private:
	void initWindow();

	static void frameBufferResizeCallback(GLFWwindow* window, int width, int height);

private:
	GLFWwindow* m_window;

	int m_width;
	int m_height;

	bool m_frameBufferResized = false;

	std::string m_windowName;
};

