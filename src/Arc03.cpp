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

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

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

	const std::string MODEL_PATH = "models/chalet.obj";
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
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		ARC_ASSERT(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str()));

		std::vector<Vertex> vertices;
		std::vector<u32> indices;
		std::unordered_map<Vertex, u32> uniqueVertices = {};

		for (const auto &shape : shapes)
		{
			for (const auto &index : shape.mesh.indices)
			{
				Vertex vertex = {};

				vertex.pos =
				{
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.texCoord =
				{
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.color = { 1.0f, 1.0f, 1.0f };

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<u32>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}
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
