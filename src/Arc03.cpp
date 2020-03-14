#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <optional>
#include <algorithm>
#include <cstdint>
#include <array>

#include "ArcGlobals.h"

struct Vertex
{
	glm::vec2 pos;
	glm::vec3 color;

	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}
};

const std::vector<Vertex> sVertices =
{
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<u16> sIndices =
{
	0, 1, 2, 2, 3, 0
};

class Arc03
{
	const u32 SCREEN_WIDTH = 800;
	const u32 SCREEN_HEIGHT = 600;

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
	void Run()
	{
		InitWindow();
		InitVulkan();
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

		glfwSetWindowUserPointer(mWindow, this);
		glfwSetFramebufferSizeCallback(mWindow, FramebufferResizeCallback);
	}

	static void FramebufferResizeCallback(GLFWwindow *window, int width, int height)
	{
		UNUSED(width);
		UNUSED(height);

		auto app = reinterpret_cast<Arc03 *>(glfwGetWindowUserPointer(window));
		app->mFramebufferResized = true;
	}

	void InitVulkan()
	{
		CreateInstance();
		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateGraphicsPipeline();
		CreateFramebuffers();
		CreateCommandPool();
		CreateVertexBuffer();
		CreateIndexBuffer();
		CreateCommandBuffers();
		CreateSyncObjects();
	}

	void CreateInstance()
	{
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

		if (mEnableValidationLayers)
		{
			VK_ASSERT(!CheckValidationLayerSupport());

			createInfo.enabledLayerCount = static_cast<u32>(mValidationLayers.size());
			createInfo.ppEnabledLayerNames = mValidationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Arc03";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;
		createInfo.pApplicationInfo = &appInfo;

		u32 glfwExtensionCount = 0;
		const char **glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;

		VK_ASSERT(vkCreateInstance(&createInfo, nullptr, &mInstance));

		u32 extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		std::cout << "Available extensions:" << std::endl;
		for (const auto &extension : extensions)
		{
			std::cout << "\t" << extension.extensionName << std::endl;
		}
	}

	void CreateSurface()
	{
		VK_ASSERT(glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface));
	}

	void PickPhysicalDevice()
	{
		mPhysicalDevice = VK_NULL_HANDLE;

		u32 deviceCount = 0;
		vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);
		ARC_ASSERT(deviceCount != 0);

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

		for (const auto &device : devices)
		{
			if (IsDeviceSuitable(device))
			{
				mPhysicalDevice = device;
				break;
			}
		}

		ARC_ASSERT(mPhysicalDevice != VK_NULL_HANDLE);
	}

	bool IsDeviceSuitable(VkPhysicalDevice device)
	{
		// Check extensions
		const bool extensionsSupported = CheckDeviceExtensionSupport(device);
		if (!extensionsSupported)
			return false;

		const SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
		const bool swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		if (!swapChainAdequate)
			return false;

		// Check for queue support
		QueueFamilyIndices indices = FindQueueFamilies(device);
		if (!indices.IsComplete())
			return false;

		return true;
	}

	bool CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		u32 extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(mDeviceExtensions.begin(), mDeviceExtensions.end());
		for (const auto &extension : availableExtensions)
			requiredExtensions.erase(extension.extensionName);

		return requiredExtensions.empty();
	}

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		u32 queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto &queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);
			if (presentSupport)
			{
				indices.presentFamily = i;
			}

			if (indices.IsComplete())
				break;

			++i;
		}

		return indices;
	}

	void CreateLogicalDevice()
	{
		const QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<u32> uniqueQueueFamilies =
		{
			indices.graphicsFamily.value(),
			indices.presentFamily.value()
		};
		const float queuePriority = 1.0f;
		for (const u32 queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<u32>(mDeviceExtensions.size());
		createInfo.ppEnabledExtensionNames = mDeviceExtensions.data();

		if (mEnableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<u32>(mValidationLayers.size());
			createInfo.ppEnabledLayerNames = mValidationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		VK_ASSERT(vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice));

		// Retrieve queues
		vkGetDeviceQueue(mDevice, indices.graphicsFamily.value(), 0, &mGraphicsQueue);
		vkGetDeviceQueue(mDevice, indices.presentFamily.value(), 0, &mPresentQueue);
	}

	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mSurface, &details.capabilities);

		u32 formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, nullptr);
		if (formatCount > 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, details.formats.data());
		}

		u32 presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, nullptr);
		if (presentModeCount > 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	void CreateSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(mPhysicalDevice);

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

		u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.minImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
			imageCount = swapChainSupport.capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = mSurface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice);
		u32 queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VK_ASSERT(vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapChain));

		vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, nullptr);
		mSwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, mSwapChainImages.data());
		mSwapChainImageFormat = surfaceFormat.format;
		mSwapChainExtent = extent;
	}

	void RecreateSwapChain()
	{
		// If minimized just wait until window is restored
		int width = 0;
		int height = 0;
		glfwGetFramebufferSize(mWindow, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(mWindow, &width, &height);
			glfwWaitEvents();
		}

		// Wait until resources are not in use
		vkDeviceWaitIdle(mDevice);

		CleanUpSwapChain();

		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateGraphicsPipeline();
		CreateFramebuffers();
		CreateCommandBuffers();
	}

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
	{
		for (const auto &availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;
		}
		return availableFormats[0];
	}

	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
	{
		for (const auto &availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				return availablePresentMode;
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(mWindow, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<u32>(width),
				static_cast<u32>(height)
			};

			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	void CreateImageViews()
	{
		mSwapChainImageViews.resize(mSwapChainImages.size());
		for (size_t i = 0; i < mSwapChainImages.size(); ++i)
		{
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = mSwapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = mSwapChainImageFormat;

			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			VK_ASSERT(vkCreateImageView(mDevice, &createInfo, nullptr, &mSwapChainImageViews[i]));
		}
	}

	void CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = mSwapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VK_ASSERT(vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &mRenderPass));
	}

	void CreateGraphicsPipeline()
	{
		std::vector<char> vertShaderCode = ReadFile("shaders/vert.spv");
		std::vector<char> fragShaderCode = ReadFile("shaders/frag.spv");

		VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		auto bindingDescription = Vertex::GetBindingDescription();
		auto attributeDescriptions = Vertex::GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<u32>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(mSwapChainExtent.width);
		viewport.height = static_cast<float>(mSwapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = mSwapChainExtent;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		VK_ASSERT(vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout));

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr;

		pipelineInfo.layout = mPipelineLayout;
		pipelineInfo.renderPass = mRenderPass;
		pipelineInfo.subpass = 0;

		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		VK_ASSERT(vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mGraphicsPipeline));

		vkDestroyShaderModule(mDevice, vertShaderModule, nullptr);
		vkDestroyShaderModule(mDevice, fragShaderModule, nullptr);
	}

	VkShaderModule CreateShaderModule(const std::vector<char> &code)
	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const u32 *>(code.data());

		VkShaderModule shaderModule;
		VK_ASSERT(vkCreateShaderModule(mDevice, &createInfo, nullptr, &shaderModule));

		return shaderModule;
	}

	void CreateFramebuffers()
	{
		mSwapChainFramebuffers.resize(mSwapChainImageViews.size());
		for (size_t i = 0; i < mSwapChainImageViews.size(); ++i)
		{
			VkImageView attachments[] = {
				mSwapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = mRenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = mSwapChainExtent.width;
			framebufferInfo.height = mSwapChainExtent.height;
			framebufferInfo.layers = 1;

			VK_ASSERT(vkCreateFramebuffer(mDevice, &framebufferInfo, nullptr, &mSwapChainFramebuffers[i]));
		}
	}

	void CreateCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(mPhysicalDevice);

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = 0;

		VK_ASSERT(vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPool));
	}

	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory)
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_ASSERT(vkCreateBuffer(mDevice, &bufferInfo, nullptr, &buffer));

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(mDevice, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		VK_ASSERT(vkAllocateMemory(mDevice, &allocInfo, nullptr, &bufferMemory));

		vkBindBufferMemory(mDevice, buffer, bufferMemory, 0);
	}

	void CreateVertexBuffer()
	{
		const VkDeviceSize bufferSize = sizeof(sVertices[0]) * sVertices.size();

		// Create a local buffer
		const VkMemoryPropertyFlags stagingBufferProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBufferProperties, stagingBuffer, stagingBufferMemory);

		// Copy vertex data into local buffer
		void *data;
		vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, sVertices.data(), (size_t)bufferSize);
		vkUnmapMemory(mDevice, stagingBufferMemory);

		// Allocate device buffer
		const VkMemoryPropertyFlags vertexBufferProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBufferProperties, mVertexBuffer, mVertexBufferMemory);

		// Copy over
		CopyBuffer(stagingBuffer, mVertexBuffer, bufferSize);

		// Free local buffer
		vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
		vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
	}

	void CreateIndexBuffer()
	{
		const VkDeviceSize bufferSize = sizeof(sIndices[0]) * sIndices.size();

		// Create a local buffer
		const VkMemoryPropertyFlags stagingBufferProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBufferProperties, stagingBuffer, stagingBufferMemory);

		// Copy index data into local buffer
		void *data;
		vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, sIndices.data(), (size_t)bufferSize);
		vkUnmapMemory(mDevice, stagingBufferMemory);

		// Allocate device buffer
		const VkMemoryPropertyFlags indexBufferProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indexBufferProperties, mIndexBuffer, mIndexBufferMemory);

		// Copy over
		CopyBuffer(stagingBuffer, mIndexBuffer, bufferSize);

		// Free local buffer
		vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
		vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
	}

	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = mCommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(mDevice, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(mGraphicsQueue);

		vkFreeCommandBuffers(mDevice, mCommandPool, 1, &commandBuffer);
	}

	u32 FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);

		for (u32 i = 0; i < memProperties.memoryTypeCount; ++i)
		{
			if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		ARC_BREAK();
		return UINT32_MAX;
	}

	void CreateCommandBuffers()
	{
		mCommandBuffers.resize(mSwapChainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = mCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<u32>(mCommandBuffers.size());

		VK_ASSERT(vkAllocateCommandBuffers(mDevice, &allocInfo, mCommandBuffers.data()));

		for (size_t i = 0; i < mCommandBuffers.size(); ++i)
		{
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0;
			beginInfo.pInheritanceInfo = nullptr;

			VK_ASSERT(vkBeginCommandBuffer(mCommandBuffers[i], &beginInfo));

			// COMMANDS
			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = mRenderPass;
			renderPassInfo.framebuffer = mSwapChainFramebuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = mSwapChainExtent;

			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(mCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);

			VkBuffer vertexBuffers[] = { mVertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(mCommandBuffers[i], 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(mCommandBuffers[i], mIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

			vkCmdDrawIndexed(mCommandBuffers[i], static_cast<u32>(sIndices.size()), 1, 0, 0, 0);
			vkCmdEndRenderPass(mCommandBuffers[i]);
			// END COMMANDS

			VK_ASSERT(vkEndCommandBuffer(mCommandBuffers[i]));
		}
	}

	void CreateSyncObjects()
	{
		mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		mImagesInFlight.resize(mSwapChainImages.size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			VK_ASSERT(vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]));
			VK_ASSERT(vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]));
			VK_ASSERT(vkCreateFence(mDevice, &fenceInfo, nullptr, &mInFlightFences[i]));
		}
	}
	void MainLoop()
	{
		while (!glfwWindowShouldClose(mWindow))
		{
			glfwPollEvents();
			DrawFrame();
		}

		vkDeviceWaitIdle(mDevice);
	}

	void DrawFrame()
	{
		// Sync
		vkWaitForFences(mDevice, 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);

		u32 imageIndex;
		VkResult result = vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			ARC_BREAK();
		}

		if (mImagesInFlight[imageIndex] != VK_NULL_HANDLE)
			vkWaitForFences(mDevice, 1, &mImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);

		mImagesInFlight[imageIndex] = mInFlightFences[mCurrentFrame];

		// Build all the required Vulkan structs...
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		// Submit
		vkResetFences(mDevice, 1, &mInFlightFences[mCurrentFrame]);
		VK_ASSERT(vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mInFlightFences[mCurrentFrame]));

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { mSwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		result = vkQueuePresentKHR(mPresentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mFramebufferResized)
		{
			mFramebufferResized = false;
			RecreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS)
		{
			ARC_BREAK();
		}

		mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void CleanUp()
	{
		CleanUpSwapChain();

		vkDestroyBuffer(mDevice, mVertexBuffer, nullptr);
		vkFreeMemory(mDevice, mVertexBufferMemory, nullptr);
		vkDestroyBuffer(mDevice, mIndexBuffer, nullptr);
		vkFreeMemory(mDevice, mIndexBufferMemory, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroySemaphore(mDevice, mRenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(mDevice, mImageAvailableSemaphores[i], nullptr);
			vkDestroyFence(mDevice, mInFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
		vkDestroyDevice(mDevice, nullptr);
		vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
		vkDestroyInstance(mInstance, nullptr);
		glfwDestroyWindow(mWindow);
		glfwTerminate();
	}

	void CleanUpSwapChain()
	{
		for (auto framebuffer : mSwapChainFramebuffers)
			vkDestroyFramebuffer(mDevice, framebuffer, nullptr);

		vkFreeCommandBuffers(mDevice, mCommandPool, static_cast<u32>(mCommandBuffers.size()), mCommandBuffers.data());

		vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
		vkDestroyRenderPass(mDevice, mRenderPass, nullptr);

		for (auto imageView : mSwapChainImageViews)
			vkDestroyImageView(mDevice, imageView, nullptr);

		vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);
	}

	bool CheckValidationLayerSupport()
	{
		u32 layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char *layerName : mValidationLayers)
		{
			bool layerFound = false;

			for (const auto &layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
				return false;
		}
		return true;
	}

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
	VkPipelineLayout mPipelineLayout;
	VkPipeline mGraphicsPipeline;
	VkCommandPool mCommandPool;
	std::vector<VkCommandBuffer> mCommandBuffers;

	// Geometry
	VkBuffer mVertexBuffer;
	VkDeviceMemory mVertexBufferMemory;
	VkBuffer mIndexBuffer;
	VkDeviceMemory mIndexBufferMemory;

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
	std::vector<VkImageView> mSwapChainImageViews;
};

int main()
{
	Arc03 app;
	app.Run();

	return EXIT_SUCCESS;
}
