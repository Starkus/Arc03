#ifndef QLIB_MEMORY_POOLALLOCATOR_H
#define QLIB_MEMORY_POOLALLOCATOR_H

#include "memory/IAllocator.h"

template<typename val_t, typename index_t = u32>
class PoolAllocator : public IAllocator
{
	static_assert(sizeof(index_t) <= sizeof(val_t), "PoolAllocator: size of value can't be smaller than the size of indices");
public:
	PoolAllocator(void *base, index_t count)
		: mBase(reinterpret_cast<val_t *>(base))
		, mCount(count)
		, mFirstFree(0)
		, mUsedCount(0)
		, mMaxUsedCount(0)
	{
		// TODO: ensure alignment? (align first block & remove 1 from total)
		for (index_t i = 0; i < count; ++i)
		{
			index_t *blockIdx = reinterpret_cast<index_t *>(&mBase[i]);
			*blockIdx = static_cast<index_t>(i + 1);
		}
	}

	val_t *Alloc()
	{
		val_t *p = nullptr;
		if (mFirstFree < mCount)
		{
			p = &mBase[mFirstFree];
			mFirstFree = *reinterpret_cast<index_t *>(p);
			++mUsedCount;
			if (mUsedCount > mMaxUsedCount)
				mMaxUsedCount = mUsedCount;
		}
		return p;
	}

	void *Alloc(size_t size) override
	{
		ARC_ASSERT(size == sizeof(val_t));
		return alloc();
	}

	void *Alloc(size_t size, u32 alignment) override
	{
		ARC_ASSERT(size == sizeof(val_t));
		ARC_ASSERT(alignment == alignof(val_t));
		return Alloc();
	}

	void *Realloc(void *p, size_t newSize) override
	{
		ARC_UNUSED(p);
		ARC_UNUSED(newSize);
		ARC_FAIL_MSG("Realloc not supported for PoolAllocator");
		return nullptr;
	}

	void *Realloc(void *p, size_t newSize, u32 alignment) override
	{
		ARC_UNUSED(p);
		ARC_UNUSED(newSize);
		ARC_UNUSED(alignment);
		ARC_FAIL_MSG("Realloc not supported for PoolAllocator");
		return nullptr;
	}

	void Dealloc(void *p) override
	{
		if (p != nullptr)
		{
			index_t blockIdx = static_cast<index_t>(PtrDiff(p, mBase) / sizeof(val_t));
			ARC_ASSERT(blockIdx >= 0 && blockIdx < mCount);
			*reinterpret_cast<index_t *>(p) = mFirstFree;
			mFirstFree = blockIdx;
			--mUsedCount;
		}
	}

	size_t GetAllocated() override
	{
		return mUsedCount;
	}

	size_t GetMaxAllocated() override
	{
		return mMaxUsedCount;
	}

private:
	val_t *mBase;
	index_t mCount;
	index_t mFirstFree;
	index_t mUsedCount;
	index_t mMaxUsedCount;
};

#endif // QLIB_MEMORY_POOLALLOCATOR_H
