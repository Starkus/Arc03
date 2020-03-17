#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
#include "Geometry.h"

class VulkanEngine
{
	struct UniformBufferObject

	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	const std::vector<const char *> mValidationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char *> mDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	const u32 MAX_FRAMES_IN_FLIGHT = 2;

	struct QueueFamilyIndices
	{
		std::optional<u32> graphicsFamily;
		std::optional<u32> presentFamily;

		bool IsComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

#ifdef ARC_DEBUG
	const bool mEnableValidationLayers = true;
#else
	const bool mEnableValidationLayers = false;
#endif

public:
	void InitVulkan(GLFWwindow *window, VkSurfaceKHR surface);
	void DrawFrame();
	void LoadTextureFromImage(void *pixels, u32 width, u32 height);
	void LoadModel(const std::vector<Vertex> &vertices, const std::vector<u32> &indices);
	void CleanUp();
	void WaitForDevice();

	static void FramebufferResizeCallback(GLFWwindow *window, int width, int height)
	{
		UNUSED(width);
		UNUSED(height);

		auto app = reinterpret_cast<VulkanEngine *>(glfwGetWindowUserPointer(window));
		app->mFramebufferResized = true;
	}

private:
	void CreateInstance();
	void CreateSurface();
	void PickPhysicalDevice();
	bool IsDeviceSuitable(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	void CreateLogicalDevice();
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	void CreateSwapChain();
	void RecreateSwapChain();
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
	void CreateSwapChainImageViews();
	void CreateRenderPass();
	void CreateDescriptorSetLayout();
	void CreateGraphicsPipeline();
	VkShaderModule CreateShaderModule(const std::vector<char> &code);
	void CreateFramebuffers();
	void CreateCommandPool();
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
			VkBuffer &buffer, VkDeviceMemory &bufferMemory);
	void CreateDepthResources();
	VkFormat FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat FindDepthFormat();
	bool HasStencilComponent(VkFormat format);
	void CreateTextureImage(u32 width, u32 height);
	void CreateImage(u32 width, u32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);
	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void CreateTextureImageView();
	void CreateTextureSampler();
	void CreateVertexBuffer();
	void FillVertexBuffer(void *dataSrc, size_t dataSize);
	void CreateIndexBuffer();
	void FillIndexBuffer(void *dataSrc, size_t dataSize);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, u32 width, u32 height);
	u32 FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties);
	void CreateUniformBuffers();
	void CreateDescriptorPool();
	void CreateDescriptorSets();
	void UpdateDescriptorSets();
	void CreateCommandBuffers();
	void CreateSyncObjects();
	void UpdateUniformBuffer(u32 currentImage);
	void CleanUpSwapChain();
	bool CheckValidationLayerSupport();

	static std::vector<char> ReadFile(const std::string &filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		ARC_ASSERT(file.is_open());

		const size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();
		return buffer;
	}

	VkInstance mInstance;
	VkPhysicalDevice mPhysicalDevice;
	VkDevice mDevice;
	VkQueue mGraphicsQueue;
	VkQueue mPresentQueue;
	VkSwapchainKHR mSwapChain;
	std::vector<VkImage> mSwapChainImages;
	VkFormat mSwapChainImageFormat;
	VkExtent2D mSwapChainExtent;
	std::vector<VkFramebuffer> mSwapChainFramebuffers;
	VkRenderPass mRenderPass;
	VkDescriptorSetLayout mDescriptorSetLayout;
	VkPipelineLayout mPipelineLayout;
	VkPipeline mGraphicsPipeline;
	VkCommandPool mCommandPool;
	std::vector<VkCommandBuffer> mCommandBuffers;
	std::vector<VkImageView> mSwapChainImageViews;

	// Geometry
	VkBuffer mVertexBuffer;
	VkDeviceMemory mVertexBufferMemory;
	VkBuffer mIndexBuffer;
	VkDeviceMemory mIndexBufferMemory;
	size_t mIndexCount;

	std::vector<VkBuffer> mUniformBuffers;
	std::vector<VkDeviceMemory> mUniformBuffersMemory;
	VkDescriptorPool mDescriptorPool;
	std::vector<VkDescriptorSet> mDescriptorSets;

	VkImage mTextureImage;
	VkDeviceMemory mTextureImageMemory;
	VkImageView mTextureImageView;
	VkSampler mTextureSampler;

	// Depth buffer
	VkImage mDepthImage;
	VkDeviceMemory mDepthImageMemory;
	VkImageView mDepthImageView;

	// Swap chain synchronization
	std::vector<VkSemaphore> mImageAvailableSemaphores;
	std::vector<VkSemaphore> mRenderFinishedSemaphores;
	std::vector<VkFence> mInFlightFences;
	std::vector<VkFence> mImagesInFlight;
	size_t mCurrentFrame = 0;

	bool mFramebufferResized = false;

	// Window
	GLFWwindow *mWindow;
	VkSurfaceKHR mSurface;
};
