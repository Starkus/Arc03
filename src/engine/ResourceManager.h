#pragma once

#include "ArcGlobals.h"
#include "engine/GraphicResource.h"
#include "memory/Memory.h"
#include "memory/GpuAllocator.h"

#include <vector>

class VulkanEngine;
class Resource;
struct ResourceHeader;

class ResourceManager
{
	ARC_DEFINE_SINGLETON(ResourceManager);

protected:
	ResourceManager() = default;

	VulkanEngine *mVulkanEngine;
	GpuAllocator mGpuVertexAllocator { VERTEX_BUFFER_SIZE };
	GpuAllocator mGpuIndexAllocator { INDEX_BUFFER_SIZE };

	std::vector<GraphicResource> mGraphicResources;

public:
	static void Initialize();
	Resource *AllocateResource(ResourceHeader header);
	Resource *LoadResource(std::string filename);
	GpuAllocator &GetVertexAllocator() { return mGpuVertexAllocator; }
	GpuAllocator &GetIndexAllocator() { return mGpuIndexAllocator; }
};
