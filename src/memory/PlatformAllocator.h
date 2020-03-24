#ifndef ENGINE_MEMORY_PLATFORMALLOCATOR_H
#define ENGINE_MEMORY_PLATFORMALLOCATOR_H

#include "Engine/Engine.h"

#include "qlib/Memory/IAllocator.h"

class PlatformAllocator : public IAllocator
{
public:
	PlatformAllocator() = default;

	void *Alloc(size_t size) override;
	void *Alloc(size_t size, u32 alignment) override;

	void *Realloc(void *p, size_t newSize) override;
	void *Realloc(void *p, size_t newSize, u32 alignment) override;

	void Dealloc(void *p) override;

	size_t GetAllocated() override;
	size_t GetMaxAllocated() override;
};

#endif // ENGINE_MEMORY_PLATFORMALLOCATOR_H
