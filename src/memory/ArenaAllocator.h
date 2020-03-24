#ifndef QLIB_MEMORY_ARENAALLOCATOR_H
#define QLIB_MEMORY_ARENAALLOCATOR_H

#include "qlib/qlib.h"
#include "qlib/Memory/IAllocator.h"

class QE_API ArenaAllocator : public IAllocator
{
public:
	ArenaAllocator(void *base, size_t size);

	void *Alloc(size_t size) override;
	void *Alloc(size_t size, u32 alignment) override;

	void *Realloc(void *p, size_t newSize) override;
	void *Realloc(void *p, size_t newSize, u32 alignment) override;

	void Dealloc(void *p) override;

	size_t GetAllocated() override;
	size_t GetMaxAllocated() override;

	void Clear();

private:
	void *mBase;
	size_t mTotal;
	size_t mUsed;
	size_t mMaxUsed;
};

#endif // QLIB_MEMORY_ARENAALLOCATOR_H
