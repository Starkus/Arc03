#pragma once

#include "VulkanEngine.h"
#include "Memory.h"
#include "ArcGlobals.h"
#include "GpuAllocator.h"

#include <vector>

struct GraphicResource
{
	size_t mOffset;
	size_t mSize;
	u32 mVertexCount;
	u32 mIndexCount;
};

class ResourceManager
{
	VulkanEngine *mVulkanEngine;
	GpuAllocator mGpuVertexAllocator { VERTEX_BUFFER_SIZE };
	GpuAllocator mGpuIndexAllocator { INDEX_BUFFER_SIZE };

public:
	ResourceManager(VulkanEngine *vulkanEngine) : mVulkanEngine(vulkanEngine) {}
	GraphicResource LoadGraphicResource(std::string filename);
};
