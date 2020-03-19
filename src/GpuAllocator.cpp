#include "GpuAllocator.h"

size_t GpuAllocator::Allocate(size_t size)
{
	size_t start = mUsed;
	mUsed += size;
	return start;
}

