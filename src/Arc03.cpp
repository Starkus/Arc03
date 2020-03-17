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

struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

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
		InitVulkanEngine();

		LoadTexture();
		LoadModel();

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

	void LoadTexture()
	{
		int texWidth, texHeight, texChannels;
		stbi_uc *pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		ARC_ASSERT(pixels);

		mVulkanEngine.LoadTextureFromImage(pixels, static_cast<u32>(texWidth), static_cast<u32>(texHeight));

		stbi_image_free(pixels);
	}

	void LoadModel()
	{
		std::vector<Vertex> vertices;
		std::vector<u32> indices;

		std::ifstream file;
		file.open(MODEL_PATH.c_str(), std::ios::binary);

		char header[9];
		file.read(header, 8);
		header[8] = 0;

		u32 vertCount, indexCount;
		file.read(reinterpret_cast<char *>(&vertCount), sizeof(u32));
		file.read(reinterpret_cast<char *>(&indexCount), sizeof(u32));

		vertices.resize(vertCount);
		indices.resize(indexCount);

		file.read(reinterpret_cast<char *>(vertices.data()), sizeof(Vertex) * vertCount);
		file.read(reinterpret_cast<char *>(indices.data()), sizeof(u32) * indexCount);

		file.close();

		mVulkanEngine.LoadModel(vertices, indices);
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
