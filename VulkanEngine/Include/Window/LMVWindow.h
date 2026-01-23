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

	VkExtent2D getExtent() { return { static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height) }; }

	void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

private:
	void initWindow();

private:
	GLFWwindow* m_window;

	const int m_width;
	const int m_height;

	std::string m_windowName;
};

