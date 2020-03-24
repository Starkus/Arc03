#include "Engine/Memory/PlatformAllocator.h"

#include <SDL2/SDL_stdinc.h>

void *PlatformAllocator::Alloc(size_t size)
{
	return SDL_malloc(size);
}

void *PlatformAllocator::Alloc(size_t size, u32 alignment)
{
	QE_UNUSED(alignment);
	return SDL_malloc(size);
}

void *PlatformAllocator::Realloc(void *p, size_t newSize)
{
	return SDL_realloc(p, newSize);
}

void *PlatformAllocator::Realloc(void *p, size_t newSize, u32 alignment)
{
	QE_UNUSED(alignment);
	return SDL_realloc(p, newSize);
}

void PlatformAllocator::Dealloc(void *p)
{
	SDL_free(p);
}

size_t PlatformAllocator::GetAllocated()
{
	return 0;
}

size_t PlatformAllocator::GetMaxAllocated()
{
	return 0;
}
