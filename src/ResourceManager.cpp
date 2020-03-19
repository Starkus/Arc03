#include "ResourceManager.h"

#include <iostream>
#include <fstream>

GraphicResource ResourceManager::LoadGraphicResource(std::string filename)
{
	std::vector<Vertex> vertices;
	std::vector<u32> indices;

	std::ifstream file;
	file.open(filename.c_str(), std::ios::binary);

	char header[9];
	file.read(header, 8);
	header[8] = 0;

	u32 vertCount, indexCount;
	file.read(reinterpret_cast<char *>(&vertCount), sizeof(u32));
	file.read(reinterpret_cast<char *>(&indexCount), sizeof(u32));

	vertices.resize(vertCount);
	indices.resize(indexCount);

	const size_t vertexDataSize = sizeof(Vertex) * vertCount;
	const size_t indexDataSize = sizeof(u32) * indexCount;
	file.read(reinterpret_cast<char *>(vertices.data()), vertexDataSize);
	file.read(reinterpret_cast<char *>(indices.data()), indexDataSize);

	file.close();

	size_t vbOffset = mGpuVertexAllocator.Allocate(vertexDataSize);
	mVulkanEngine->FillVertexBuffer((void *)vertices.data(), vbOffset, vertexDataSize);

	size_t ibOffset = mGpuIndexAllocator.Allocate(indexDataSize);
	mVulkanEngine->FillIndexBuffer((void *)indices.data(), ibOffset, indexDataSize);

	GraphicResource res = { ibOffset, indexDataSize, vertCount, indexCount };
	return res;
}
