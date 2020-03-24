#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#pragma warning(push, 1) // Dumb glm giving warnings
#include <glm/gtx/hash.hpp>
#pragma warning(pop)

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <optional>
#include <algorithm>
#include <cstdint>
#include <array>
#include <unordered_map>

#include "ArcGlobals.h"
#include "VulkanEngine.h"
#include "Geometry.h"
#include "Memory.h"
#include "ResourceManager.h"
#include "ComponentManager.h"

class Arc03
{
	const u32 SCREEN_WIDTH = 800;
	const u32 SCREEN_HEIGHT = 600;

	const std::string MODEL_PATH = "models/chalet.bin";
	const std::string TEXTURE_PATH = "textures/chalet.jpg";

public:
	Arc03() :
		mVulkanEngine(&mComponentManager),
		mResourceManager(&mVulkanEngine),
		mComponentManager(&mResourceManager)
	{
	}

	void Run()
	{
		InitWindow();
		InitVulkanEngine();

		LoadTexture(TEXTURE_PATH);
		LoadTexture("textures/texture.jpg");
		LoadTexture("textures/gradient.png");
		LoadTexture("textures/bricks.png");

		mComponentManager.CreateGraphicComponent(MODEL_PATH);
		mComponentManager.CreateGraphicComponent("models/monkey.bin");

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

		glfwSetWindowUserPointer(mWindow, &mVulkanEngine);
		glfwSetFramebufferSizeCallback(mWindow, VulkanEngine::FramebufferResizeCallback);
	}

	void InitVulkanEngine()
	{
		mVulkanEngine.InitVulkan(mWindow, mSurface);
	}

	void LoadTexture(std::string textureFilename)
	{
		int texWidth, texHeight, texChannels;
		stbi_uc *pixels = stbi_load(textureFilename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		ARC_ASSERT(pixels);

		mVulkanEngine.LoadTextureFromImage(pixels, static_cast<u32>(texWidth), static_cast<u32>(texHeight));

		stbi_image_free(pixels);
	}

	void MainLoop()
	{
		while (!glfwWindowShouldClose(mWindow))
		{
			glfwPollEvents();
			mVulkanEngine.DrawFrame();
		}

		mVulkanEngine.WaitForDevice();
	}

	void CleanUp()
	{
		mVulkanEngine.CleanUp();

		glfwDestroyWindow(mWindow);
		glfwTerminate();
	}

	VulkanEngine mVulkanEngine;
	ResourceManager mResourceManager;
	ComponentManager mComponentManager;

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
