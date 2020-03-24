#pragma once

class GpuAllocator
{
	const size_t mBufferSize;
	size_t mUsed;

public:
	GpuAllocator(size_t bufferSize) : mBufferSize(bufferSize), mUsed(0) {}
	size_t Allocate(size_t size);
};

