#ifndef QLIB_MEMORY_IALLOCATOR_H
#define QLIB_MEMORY_IALLOCATOR_H

class IAllocator
{
public:
	IAllocator() = default;
	virtual ~IAllocator() = default;

	virtual void *Alloc(size_t size) = 0;
	virtual void *Alloc(size_t size, u32 alignment) = 0;

	virtual void *Realloc(void *p, size_t newSize) = 0;
	virtual void *Realloc(void *p, size_t newSize, u32 alignment) = 0;

	virtual void Dealloc(void *p) = 0;

	virtual size_t GetAllocated() = 0;
	virtual size_t GetMaxAllocated() = 0;

	template<typename val_t>
	val_t *Alloc()
	{
		return static_cast<val_t*>(Alloc(sizeof(val_t), alignof(val_t)));
	}

	template<typename val_t>
	val_t *AllocArray(size_t count)
	{
		return static_cast<val_t*>(Alloc(sizeof(val_t) * count, alignof(val_t)));
	}

	template<typename val_t>
	val_t *ReallocArray(void *p, size_t count)
	{
		return static_cast<val_t*>(Realloc(p, sizeof(val_t) * count, alignof(val_t)));
	}

	template<typename val_t>
	val_t *AllocNew()
	{
		val_t *p = Alloc<val_t>();
		new (p) val_t();
		return p;
	}

	template<typename val_t, typename... Args>
	val_t *AllocNew(Args... args)
	{
		val_t *p = Alloc<val_t>();
		new (p) val_t(args...);
		return p;
	}

	template<typename val_t>
	val_t *AllocNewArray(size_t count)
	{
		val_t *p = AllocArray<val_t>(count);
		for (size_t i = 0; i < count; ++i)
			new (&p[i]) val_t();
		return p;
	}

	template<typename val_t>
	void DeallocDelete(val_t *p)
	{
		if (p)
		{
			p->~val_t();
			Dealloc(p);
		}
	}

	template<typename val_t>
	void DeallocDeleteArray(val_t *p, size_t count)
	{
		if (p)
		{
			for (size_t i = 0; i < count; ++i)
			{
				p[i].~val_t();
			}
			Dealloc(p);
		}
	}
};

inline void *operator new(size_t size, IAllocator &a)
{
	return a.Alloc(size);
}

inline void *operator new[](size_t size, IAllocator &a)
{
	return a.Alloc(size);
}

#endif // QLIB_MEMORY_IALLOCATOR_H
