#pragma once

#include "ArcGlobals.h"
#include "memory/Memory.h"
#include "memory/GpuAllocator.h"
#include "engine/Resource.h"

class GraphicResource : public Resource
{
	u64 mIndexBufferOffset;
	u32 mVertexCount;
	u32 mIndexCount;

public:
	u64 GetIndexBufferOffset() const { return mIndexBufferOffset; }
	u32 GetVertexCount() const { return mVertexCount; }
	u32 GetIndexCount() const { return mIndexCount; }
	u64 GetIndexDataSize() const { return mIndexCount * sizeof(u32); }

protected:
	void Load(void *data, u64 dataSize) final;
};
