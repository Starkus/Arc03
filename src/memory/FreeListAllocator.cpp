#include "qlib/Memory/FreeListAllocator.h"

#include "qlib/Assert.h"
#include "qlib/Memory/Memory.h"

FreeListAllocator::FreeListAllocator()
	: mFreeBlocks(nullptr)
	, mAllocated(0)
	, mMaxAllocated(0)
{}

FreeListAllocator::FreeListAllocator(void *base, u32 size)
	: mFreeBlocks(nullptr)
	, mAllocated(0)
	, mMaxAllocated(0)
{
	Initialize(base, size);
}

void FreeListAllocator::Initialize(void *base, u32 size)
{
	QE_ASSERT(size > sizeof(FreeBlock));
	mFreeBlocks = reinterpret_cast<FreeBlock *>(base);
	mFreeBlocks->size = static_cast<u32>(size);
	mFreeBlocks->next = nullptr;
	mAllocated = 0;
}

void *FreeListAllocator::Alloc(size_t size)
{
	return Alloc(size, 1);
}

void *FreeListAllocator::Alloc(size_t size, u32 alignment)
{
	FreeBlock *prevBlock;
	FreeBlock *block = GetFreeBlock(&prevBlock, size, alignment);
	return (block != nullptr) ? AllocOnBlock(block, prevBlock, size, alignment) : nullptr;
}

void *FreeListAllocator::Realloc(void *p, size_t newSize)
{
	return Realloc(p, newSize, 1);
}

void *FreeListAllocator::Realloc(void *p, size_t newSize, u32 alignment)
{
	if (p == nullptr)
		return Alloc(newSize, alignment);

	void *ptr = nullptr;

	AllocationHeader *header = reinterpret_cast<AllocationHeader *>(PtrSub(p, sizeof(AllocationHeader)));
	u32 prevSize = header->size - header->offset;
	if (newSize > prevSize)
	{
		if (GrowBlock(p, newSize))
		{
			ptr = p;
		}
		else
		{
			// TODO: expand block if possible (both ways)
			ptr = Alloc(newSize, alignment);
			if (ptr != nullptr)
			{
				MemCopy(ptr, p, header->size);
				Dealloc(p);
			}
		}
	}
	else
	{
		if (newSize < prevSize)
			ShrinkBlock(p, newSize);
		ptr = p;
	}
	return ptr;
}

void FreeListAllocator::Dealloc(void *p)
{
	if (p != nullptr)
	{
		const AllocationHeader *header = reinterpret_cast<AllocationHeader *>(PtrSub(p, sizeof(AllocationHeader)));
		void *blockStart = PtrSub(p, header->offset);
		void *blockEnd = PtrAdd(blockStart, header->size);

		FreeBlock *newBlock = static_cast<FreeBlock *>(blockStart);
		newBlock->size = header->size;

		FreeBlock *prevBlock = nullptr;
		FreeBlock *nextBlock = mFreeBlocks;
		while ((nextBlock != nullptr) && (nextBlock < blockEnd))
		{
			prevBlock = nextBlock;
			nextBlock = nextBlock->next;
		}

		if (prevBlock == nullptr)
		{
			mFreeBlocks = newBlock;
		}
		else if (PtrAdd(prevBlock, prevBlock->size) == blockStart)
		{
			prevBlock->size += newBlock->size;
			newBlock = prevBlock;
		}
		else
		{
			prevBlock->next = newBlock;
		}

		if ((nextBlock != nullptr) && (nextBlock == blockEnd))
		{
			newBlock->size += nextBlock->size;
			newBlock->next = nextBlock->next;
		}
		else
		{
			newBlock->next = nextBlock;
		}

		mAllocated -= header->size;
	}
}

size_t FreeListAllocator::GetAllocated()
{
	return mAllocated;
}

size_t FreeListAllocator::GetMaxAllocated()
{
	return mMaxAllocated;
}

FreeListAllocator::FreeBlock *FreeListAllocator::GetFreeBlock(FreeBlock **prevBlock, size_t size, u32 alignment)
{
	// TODO: search for best-fit?
	*prevBlock = nullptr;
	FreeBlock *block = mFreeBlocks;
	while (block != nullptr)
	{
		u32 offset = AlignOffsetExtra(block, alignment, sizeof(AllocationHeader));
		u32 totalSize = static_cast<u32>(size) + offset;
		if (block->size >= totalSize)
		{
			break;
		}
		else
		{
			*prevBlock = block;
			block = block->next;
		}
	}
	return block;
}

void *FreeListAllocator::AllocOnBlock(FreeBlock *block, FreeBlock *prevBlock, size_t size, u32 alignment)
{
	u32 offset = AlignOffsetExtra(block, alignment, sizeof(AllocationHeader));
	u32 totalSize = static_cast<u32>(size) + offset;
	if ((block->size - totalSize) < sizeof(FreeBlock))
	{
		totalSize = block->size;
		if (prevBlock != nullptr)
			prevBlock->next = block->next;
		else
			mFreeBlocks = block->next;
	}
	else
	{
		u32 oldSize = block->size;
		FreeBlock *nextBlock = block->next;
		FreeBlock *newBlock = reinterpret_cast<FreeBlock *>(PtrAdd(block, totalSize));
		newBlock->size = oldSize - totalSize;
		newBlock->next = nextBlock;
		if (prevBlock != nullptr)
			prevBlock->next = newBlock;
		else
			mFreeBlocks = newBlock;
	}
	void *p = PtrAdd(block, offset);
	AllocationHeader *header = reinterpret_cast<AllocationHeader *>(PtrSub(p, sizeof(AllocationHeader)));
	header->size = totalSize;
	header->offset = static_cast<u8>(offset);
	mAllocated += totalSize;
	if (mAllocated > mMaxAllocated)
		mMaxAllocated = mAllocated;
	return p;
}

FreeListAllocator::FreeBlock *FreeListAllocator::GetNextContiguousBlock(void *p, FreeBlock **prevBlock)
{
	const AllocationHeader *header = reinterpret_cast<AllocationHeader *>(PtrSub(p, sizeof(AllocationHeader)));
	void *blockEnd = PtrAdd(p, header->size - header->offset);

	FreeBlock *prev = nullptr;
	FreeBlock *next = mFreeBlocks;
	while ((next != nullptr) && (next < blockEnd))
	{
		prev = next;
		next = next->next;
	}

	if (prevBlock != nullptr)
		*prevBlock = prev;

	return (next == blockEnd) ? next : nullptr;
}

bool FreeListAllocator::GrowBlock(void *p, size_t size)
{
	FreeBlock *prevBlock;
	FreeBlock *nextBlock = GetNextContiguousBlock(p, &prevBlock);
	if (nextBlock == nullptr)
		return false;

	AllocationHeader *header = reinterpret_cast<AllocationHeader *>(PtrSub(p, sizeof(AllocationHeader)));
	u32 newSize = static_cast<u32>(size);
	u32 oldTotalSize = header->size;
	u32 oldAllocatedSize = oldTotalSize - header->offset;
	u32 extraSize = newSize - oldAllocatedSize;
	if (nextBlock->size < extraSize)
		return false;

	u32 newTotalSize = newSize + header->offset;
	u32 nextBlockSize = nextBlock->size - extraSize;
	if (nextBlockSize < sizeof(FreeBlock))
	{
		newTotalSize = header->size + nextBlock->size;
		if (prevBlock != nullptr)
			prevBlock->next = nextBlock->next;
		else
			mFreeBlocks = nextBlock->next;
	}
	else
	{
		FreeBlock *next = nextBlock->next;
		FreeBlock *newBlock = reinterpret_cast<FreeBlock *>(PtrAdd(nextBlock, extraSize));
		newBlock->size = nextBlockSize;
		newBlock->next = next;
		if (prevBlock != nullptr)
			prevBlock->next = newBlock;
		else
			mFreeBlocks = newBlock;
	}

	header->size = newTotalSize;
	mAllocated += (newTotalSize - oldTotalSize);
	if (mAllocated > mMaxAllocated)
		mMaxAllocated = mAllocated;

	return true;
}

void FreeListAllocator::ShrinkBlock(void *p, size_t size)
{
	u32 newSize = static_cast<u32>(size);
	AllocationHeader *header = reinterpret_cast<AllocationHeader *>(PtrSub(p, sizeof(AllocationHeader)));
	void *blockStart = PtrSub(p, header->offset);
	u32 oldTotalSize = header->size;
	u32 oldAllocatedSize = oldTotalSize - header->offset;
	u32 freeSize = oldAllocatedSize - static_cast<u32>(newSize);

	FreeBlock *prevBlock = nullptr;
	FreeBlock *nextBlock = mFreeBlocks;
	while ((nextBlock != nullptr) && (nextBlock < blockStart))
	{
		prevBlock = nextBlock;
		nextBlock = nextBlock->next;
	}

	u32 newTotalSize = oldTotalSize;
	if ((nextBlock != nullptr) && (nextBlock == blockStart))
	{
		newTotalSize = header->offset + newSize;
		header->size = newTotalSize;

		u32 newBlockSize = nextBlock->size + freeSize;
		FreeBlock *next = nextBlock->next;
		FreeBlock *newBlock = reinterpret_cast<FreeBlock *>(PtrAdd(blockStart, newTotalSize));
		newBlock->size = newBlockSize;
		newBlock->next = next;
		if (prevBlock != nullptr)
			prevBlock->next = newBlock;
		else
			mFreeBlocks = newBlock;
	}
	else if (freeSize >= sizeof(FreeBlock))
	{
		newTotalSize = header->offset + newSize;
		header->size = newTotalSize;

		FreeBlock *newBlock = reinterpret_cast<FreeBlock *>(PtrAdd(blockStart, newTotalSize));
		newBlock->size = freeSize;
		newBlock->next = nextBlock;
		if (prevBlock != nullptr)
			prevBlock->next = newBlock;
		else
			mFreeBlocks = newBlock;
	}

	mAllocated -= (oldTotalSize - newTotalSize);
}
