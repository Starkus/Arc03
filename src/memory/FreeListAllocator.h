#ifndef QLIB_MEMORY_FREELISTALLOCATOR_H
#define QLIB_MEMORY_FREELISTALLOCATOR_H

#include "qlib/qlib.h"
#include "qlib/Memory/IAllocator.h"

class QE_API FreeListAllocator : public IAllocator
{
public:
	FreeListAllocator();
	FreeListAllocator(void *base, u32 size);

	void Initialize(void *base, u32 size);

	void *Alloc(size_t size) override;
	void *Alloc(size_t size, u32 alignment) override;

	void *Realloc(void *p, size_t newSize) override;
	void *Realloc(void *p, size_t newSize, u32 alignment) override;

	void Dealloc(void *p) override;

	size_t GetAllocated() override;
	size_t GetMaxAllocated() override;

private:
	struct AllocationHeader
	{
		u32 size;
		u8 offset;
	};

	struct FreeBlock
	{
		u32 size;
		FreeBlock *next; // TODO: replace with offset?
	};

	FreeBlock *mFreeBlocks;
	u32 mAllocated;
	u32 mMaxAllocated;

	FreeBlock *GetFreeBlock(FreeBlock **prevBlock, size_t size, u32 alignment);
	void *AllocOnBlock(FreeBlock *block, FreeBlock *prevBlock, size_t size, u32 alignment);

	FreeBlock *GetNextContiguousBlock(void *p, FreeBlock **prevBlock = nullptr);
	bool GrowBlock(void *p, size_t newSize);
	void ShrinkBlock(void *, size_t newSize);
};

#endif // QLIB_MEMORY_FREELISTALLOCATOR_H
