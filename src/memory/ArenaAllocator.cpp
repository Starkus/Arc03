#include "qlib/Memory/ArenaAllocator.h"

#include "qlib/Assert.h"
#include "qlib/Memory/Memory.h"

ArenaAllocator::ArenaAllocator(void *base, size_t size)
	: mBase(base)
	, mTotal(size)
	, mUsed(0)
	, mMaxUsed(0)
{}

void *ArenaAllocator::Alloc(size_t size)
{
	void *p = nullptr;
	if (mUsed + size <= mTotal)
	{
		p = PtrAdd(mBase, mUsed);
		mUsed += size;
		if (mUsed > mMaxUsed)
			mMaxUsed = mUsed;
	}
	return p;
}

void *ArenaAllocator::Alloc(size_t size, u32 alignment)
{
	void *p = nullptr;
	void *top = PtrAdd(mBase, mUsed);
	const size_t offset = AlignOffset(top, alignment);
	if ((mUsed + size + offset) <= mTotal)
	{
		p = PtrAdd(top, offset);
		mUsed += size + offset;
		if (mUsed > mMaxUsed)
			mMaxUsed = mUsed;
	}
	return p;
}

void *ArenaAllocator::Realloc(void *p, size_t newSize)
{
	QE_UNUSED(p);
	QE_UNUSED(newSize);
	QE_FAIL_MSG("Realloc not supported for ArenaAllocator");
	return nullptr;
}

void *ArenaAllocator::Realloc(void *p, size_t newSize, u32 alignment)
{
	QE_UNUSED(p);
	QE_UNUSED(newSize);
	QE_UNUSED(alignment);
	QE_FAIL_MSG("Realloc not supported for ArenaAllocator");
	return nullptr;
}

void ArenaAllocator::Dealloc(void *p)
{
	QE_UNUSED(p);
	QE_FAIL_MSG("Dealloc not supported for ArenaAllocator");
}

size_t ArenaAllocator::GetAllocated()
{
	return mUsed;
}

size_t ArenaAllocator::GetMaxAllocated()
{
	return mMaxUsed;
}

void ArenaAllocator::Clear()
{
	mUsed = 0;
}
