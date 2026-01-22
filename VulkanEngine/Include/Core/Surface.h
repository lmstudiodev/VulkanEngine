#pragma once
#include <Prerequisite.h>
#include <Game/Game.h>

class Surface
{
public:
	Surface(Game* game, bool debugMode);
	~Surface();

public:
	VkSurfaceKHR getSurface();

private:
	void createSurface();

private:
	VkSurfaceKHR m_surface;

	Game* m_game = nullptr;

	bool m_debugMode;
};

