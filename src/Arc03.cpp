#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <unordered_map>
#include <vector>

#include "ArcGlobals.h"
#include "engine/ComponentManager.h"
#include "engine/ResourceManager.h"
#include "memory/Memory.h"
#include "render/VulkanEngine.h"
#include "util/Geometry.h"

class Arc03
{
	const u32 SCREEN_WIDTH = 800;
	const u32 SCREEN_HEIGHT = 600;

	const std::string MODEL_PATH = "models/chalet.bin";
	const std::string TEXTURE_PATH = "textures/chalet.jpg";

public:
	void Run()
	{
		InitWindow();

		VulkanEngine::Initialize(mWindow, mSurface);
		ResourceManager::Initialize();
		ComponentManager::Initialize();

		LoadTexture(TEXTURE_PATH);
		LoadTexture("textures/texture.jpg");
		LoadTexture("textures/gradient.png");
		LoadTexture("textures/bricks.png");
		VulkanEngine::Instance()->UpdateDescriptorSets();

		ComponentManager::Instance()->CreateGraphicComponent(MODEL_PATH);
		ComponentManager::Instance()->CreateGraphicComponent("models/monkey.bin");

		MainLoop();
		CleanUp();
	}

private:
	void InitWindow()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		mWindow = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Vulkan window", nullptr, nullptr);

		glfwSetFramebufferSizeCallback(mWindow, VulkanEngine::FramebufferResizeCallback);
	}

	void LoadTexture(std::string textureFilename)
	{
		int texWidth, texHeight, texChannels;
		stbi_uc *pixels = stbi_load(textureFilename.c_str(), &texWidth, &texHeight, &texChannels,
				STBI_rgb_alpha);
		ARC_ASSERT(pixels);

		VulkanEngine::Instance()->LoadTextureFromImage(pixels, static_cast<u32>(texWidth),
				static_cast<u32>(texHeight));

		stbi_image_free(pixels);
	}

	void MainLoop()
	{
		while (!glfwWindowShouldClose(mWindow))
		{
			glfwPollEvents();
			VulkanEngine::Instance()->DrawFrame();
		}

		VulkanEngine::Instance()->WaitForDevice();
	}

	void CleanUp()
	{
		VulkanEngine::Instance()->CleanUp();

		glfwDestroyWindow(mWindow);
		glfwTerminate();
	}

	// Window
	GLFWwindow *mWindow;
	VkSurfaceKHR mSurface;
	std::vector<VkImageView> mSwapChainImageViews;
};

int main()
{
	Arc03 app;
	app.Run();

	return EXIT_SUCCESS;
}
