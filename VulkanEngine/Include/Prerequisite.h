#pragma once
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdint>
#include <limits>
#include <algorithm>


class Device;
class Surface;
class SwapChain;
class Shader;
class GraphicPipeline;

#define VKDEBUG(message){\
std::cout << "[VULKAN ENGINE DEBUG] " << message << "\n";\
}

#define VKINFO(message){\
std::cout << "[VULKAN ENGINE INFO] " << message << "\n";\
}

#define VKWARNING(message){\
std::cout << "[VULKAN ENGINE WARNING] " << message << "\n";\
}

#define VKERROR(message){\
std::cout << "[VULKAN ENGINE ERROR] " << message << "\n";\
}

#define VKERROR_AND_THROW(message){\
std::cout << "[VULKAN ENGINE ERROR] " << message << "\n";\
throw std::runtime_error("");\
}