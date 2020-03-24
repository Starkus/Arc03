#include <iostream>
#include <fstream>

#include "render/VulkanEngine.h"

void GraphicResource::Load(void *data, u64 dataSize)
{
	ARC_UNUSED(dataSize);
	u8 *readPtr = (u8 *)data;

	mVertexCount = *(u32 *)readPtr;
	readPtr += 4;
	mIndexCount = *(u32 *)readPtr;
	readPtr += 4;

	const size_t vertexDataSize = sizeof(Vertex) * mVertexCount;
	const size_t indexDataSize = sizeof(u32) * mIndexCount;

	u64 vbOffset = ResourceManager::Instance()->GetVertexAllocator().Allocate(vertexDataSize);
	VulkanEngine::Instance()->FillVertexBuffer((void *)readPtr, vbOffset, vertexDataSize);
	readPtr += vertexDataSize;

	mIndexBufferOffset = ResourceManager::Instance()->GetIndexAllocator().Allocate(indexDataSize);
	VulkanEngine::Instance()->FillIndexBuffer((void *)readPtr, mIndexBufferOffset, indexDataSize);
}
